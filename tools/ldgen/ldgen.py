#!/usr/bin/env python
#
# Copyright 2018-2019 Espressif Systems (Shanghai) PTE LTD
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse
import sys
import tempfile
import subprocess

from fragments import FragmentFile
from sdkconfig import SDKConfig
from generation import GenerationModel, TemplateModel, SectionsInfo
from ldgen_common import LdGenFailure
from pyparsing import ParseException, ParseFatalException
from io import StringIO


def main():

    argparser = argparse.ArgumentParser(description="ESP-IDF linker script generator")

    argparser.add_argument(
        "--input", "-i",
        help="Linker template file",
        type=argparse.FileType("r"))

    argparser.add_argument(
        "--fragments", "-f",
        type=argparse.FileType("r"),
        help="Input fragment files",
        nargs="+")

    argparser.add_argument(
        "--libraries-file",
        type=argparse.FileType("r"),
        help="File that contains the list of libraries in the build")

    argparser.add_argument(
        "--output", "-o",
        help="Output linker script",
        type=str)

    argparser.add_argument(
        "--config", "-c",
        help="Project configuration")

    argparser.add_argument(
        "--kconfig", "-k",
        help="IDF Kconfig file")

    argparser.add_argument(
        "--env", "-e",
        action='append', default=[],
        help='Environment to set when evaluating the config file', metavar='NAME=VAL')

    argparser.add_argument(
        "--objdump",
        help="Path to toolchain objdump")

    args = argparser.parse_args()

    input_file = args.input
    fragment_files = [] if not args.fragments else args.fragments
    libraries_file = args.libraries_file
    config_file = args.config
    output_path = args.output
    kconfig_file = args.kconfig
    objdump = args.objdump

    try:
        sections_infos = SectionsInfo()
        for library in libraries_file:
            library = library.strip()
            if library:
                dump = StringIO(subprocess.check_output([objdump, "-h", library]).decode())
                dump.name = library
                sections_infos.add_sections_info(dump)

        generation_model = GenerationModel()

        sdkconfig = SDKConfig(kconfig_file, config_file, args.env)

        for fragment_file in fragment_files:
            try:
                fragment_file = FragmentFile(fragment_file, sdkconfig)
            except (ParseException, ParseFatalException) as e:
                # ParseException is raised on incorrect grammar
                # ParseFatalException is raised on correct grammar, but inconsistent contents (ex. duplicate
                # keys, key unsupported by fragment, unexpected number of values, etc.)
                raise LdGenFailure("failed to parse %s\n%s" % (fragment_file.name, str(e)))
            generation_model.add_fragments_from_file(fragment_file)

        mapping_rules = generation_model.generate_rules(sections_infos)

        script_model = TemplateModel(input_file)
        script_model.fill(mapping_rules)

        with tempfile.TemporaryFile("w+") as output:
            script_model.write(output)
            output.seek(0)
            with open(output_path, "w") as f:  # only create output file after generation has suceeded
                f.write(output.read())
    except LdGenFailure as e:
        print("linker script generation failed for %s\nERROR: %s" % (input_file.name, e))
        sys.exit(1)


if __name__ == "__main__":
    main()
