# ****************************************************************************************************************
# * Kira - cross-platform component-based modular application development framework written in C11               *
# *                                                                                                              *
# * (c) 2024-2025 TophUwO <tophuwo01@gmail.com>                                                                  *
# *                                                                                                              *
# * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
# * this project. If this file is not present, visit                                                             *
# *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
# ****************************************************************************************************************


from argp import *


GL_ARGSJSON: dict[str, any] = {
    "description": "generates build information, UUIDs, hashes, and more",
    "prefixes":    "-",

    "args": {
        "sbinfo": {
            "type":        "command",
            "description": "generates static build information in a way consumable by C/C++ projects",

            "args": {
                "input": {
                    "type":        "str",
                    "required":    True,
                    "description": "file to the configured static build data"
                },
                "output": {
                    "type":        "str",
                    "flag":        "o",
                    "required":    False,
                    "default":     None,
                    "description": "directory path to where the generated build information is written"
                }
            }
        },
        "uuid": {
            "type":        "command",
            "description": "generates version-4 UUIDs",

            "args": {
                "fmt": {
                    "type":        "str",
                    "flag":        "f",
                    "required":    True,
                    "description": "identifies the output format of the UUID",
                    "choices":     ["metadata", "struct"]
                },
                "number": {
                    "type":        "int",
                    "flag":        "n",
                    "required":    False,
                    "default":     1,
                    "description": "specifies how many UUIDs to generate"
                },
                "output": {
                    "type":        "str",
                    "flag":        "o",
                    "required":    False,
                    "default":     None,
                    "description": "path to the file where UUIDs are written"
                },
                "copy": {
                    "type":        "bool",
                    "flag":        "c",
                    "required":    False,
                    "default":     False,
                    "description": "whether or not to copy the result(s) to the clipboard"
                }
            }
        }
    }
}


if __name__ == '__main__':
    parser, args = ParseCommandLine(GL_ARGSJSON)

    


