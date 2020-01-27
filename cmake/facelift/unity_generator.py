#!/usr/bin/env python3

#######################################################################
##
## Copyright (C) 2019 Luxoft Sweden AB
##
## This file is part of the FaceLift project
##
## Permission is hereby granted, free of charge, to any person
## obtaining a copy of this software and associated documentation files
## (the "Software"), to deal in the Software without restriction,
## including without limitation the rights to use, copy, modify, merge,
## publish, distribute, sublicense, and/or sell copies of the Software,
## and to permit persons to whom the Software is furnished to do so,
## subject to the following conditions:
##
## The above copyright notice and this permission notice shall be
## included in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
## EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
## MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
## NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
## BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
## ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
## CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## SPDX-License-Identifier: MIT
##
#######################################################################

import os.path
import os
import sys
import json
import click

cmake_separator = ";"
internal_separator = "#"

unit_cache_file_format = "cached_values_{}.json"
unit_filename_format = "{}_unity_{}.cpp"
unit_include_limit = 5
unit_size_limit = 500000


class UnitCache:
    def __init__(self, units):
        self.units = units

    def file_list(self):
        return [file for unit in self.units for file in unit.file_list]

    def last_unit(self):
        if len(self.units):
            return max(self.units, key=lambda u: u.unit_index)
        self.units.append(Unit())
        return self.last_unit()

    def add_files(self, files):
        unit = self.last_unit()
        for file in files:
            file_size = os.path.getsize(file) if os.path.isfile(file) else 0
            if not unit.can_add_file(file_size):
                previous_index = unit.unit_index
                unit = Unit(unit_index=previous_index + 1)
                self.units.append(unit)
            unit.add_file(file, file_size)

    def remove_files(self, files):
        for unit in self.units:
            unit.remove_files(files)
        self.units[:] = [unit for unit in self.units if len(unit.file_list) > 0]

    @classmethod
    def from_json(cls, input: dict):
        units = list(map(Unit.from_json, input["units"]))
        return cls(units)


class Unit:
    def __init__(self, file_list = None, unit_index = 1):
        self.file_list = file_list if file_list is not None else []
        self.unit_index = unit_index
        self.size = 0
        self.update_size()

    def add_file(self, file, file_size):
        self.file_list.append(file)
        self.size += file_size

    def remove_files(self, files):
        self.file_list[:] = [file for file in self.file_list if file not in files]
        self.update_size()

    def sort_files(self):
        self.file_list.sort()

    def update_size(self):
        self.size = 0
        for file in self.file_list:
            if os.path.exists(file) and os.path.isfile(file):
                self.size += os.path.getsize(file) if os.path.isfile(file) else 0

    def can_add_file(self, file_size):

        # ok to add file if there no files
        if len(self.file_list) == 0:
            return True
        return len(self.file_list) < unit_include_limit and file_size + self.size < unit_size_limit

    @classmethod
    def from_json(cls, input: dict):
        return cls(file_list=input["file_list"], unit_index=input["unit_index"])


class UnityGenerator:
    @classmethod
    def generate(cls, cache: UnitCache, target_folder, target_name):
        units = []
        for unit in cache.units:
            unit_path = UnityGenerator.generate_unit(unit, target_folder, target_name)
            file_list = internal_separator.join(unit.file_list)
            units.append(internal_separator.join([unit_path, file_list]))

        return units

    @classmethod
    def generate_unit(cls, unit: Unit, target_folder, target_name):
        def write_file():
            with open(unit_path, 'w') as file:
                file.write(file_content)

        unit.sort_files()
        file_content = ""
        for file in unit.file_list:
            file_content += "#include \"{}\"\n".format(file)

        unit_path = os.path.join(target_folder, unit_filename_format.format(target_name, unit.unit_index))

        if os.path.exists(unit_path) and os.path.isfile(unit_path):
            file = open(unit_path, 'r')
            if file.read() != file_content:
                write_file()
        else:
            write_file()

        return unit_path


def run_generation(files, target_folder, target_name):

    cache = UnitCache([])

    cache_path = os.path.join(target_folder, unit_cache_file_format.format(target_name))
    if os.path.exists(cache_path) and os.path.isfile(cache_path):
        with open(cache_path) as file:
            cache = UnitCache.from_json(json.load(file))

    cached_files = cache.file_list()

    to_remove = []
    for file in cached_files:
        if file not in files:
            to_remove.append(file)

    to_add = []
    for file in files:
        if file not in cached_files:
            to_add.append(file)

    if len(to_remove):
        cache.remove_files(to_remove)
    if len(to_add):
        cache.add_files(to_add)

    generated_units = UnityGenerator.generate(cache, target_folder, target_name)

    cmake_string = ""
    if os.name == 'nt':
        # needed for cmake compatibility
        cmake_string = cmake_separator.join(generated_units).replace("\\", "/")
    else:
        cmake_string = cmake_separator.join(generated_units)

    sys.stdout.write(cmake_string)

    with open(cache_path, 'w') as file:
        json.dump(cache, file, default=lambda c: c.__dict__)


@click.command()
@click.option('--output')
@click.option('--target_name')
@click.option('--file_list', type=str)
@click.option('--size_limit', type=int)
@click.option('--separator', type=str)

def generate_units(output, target_name, file_list, size_limit = None, separator = None):

    files = []
    with open(file_list) as file:
        for line in file:
            line = line.strip()
            files.append(line)

    if size_limit is not None:
        global unit_size_limit
        unit_size_limit = size_limit

    if separator is not None:
        global internal_separator
        internal_separator = separator

    run_generation(files, output, target_name)


if __name__ == '__main__':
    generate_units()
