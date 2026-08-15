# ****************************************************************************************************************
# * Kira - cross-platform component-based modular application development framework written in C11               *
# *                                                                                                              *
# * (c) 2024-2025 Toffi <tophuwo01@gmail.com>                                                                  *
# *                                                                                                              *
# * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
# * this project. If this file is not present, visit                                                             *
# *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
# ****************************************************************************************************************


from json       import *
from jsonschema import *
from argparse   import *


# A valid command-line argument specification must adhere to the following rules:
#     (1) If 'type' of an argument is 'command', then 'args' must be one of its attributes.
#     (2) If 'type' is not 'command', 'args' must not be present.
#     (3) The type of the items of 'choices' and 'default' must be the same as 'type'.
#     (4) 'description', 'flag', and 'action' (if present) must not be empty.
#     (5) 'type' and 'description' are always required.
#     (6) If 'type' is 'command', then 'flag', 'required', 'action' and 'default' must not be present.
#     (7) If 'flag' is not present, then 'required' must be present and set to 'true'.
#     (8) If 'required' is set to 'true', then 'default' must not be present.
GL_SCHEMA_ARGSJSON: dict[str, any] = {
    "$schema":     "https://json-schema.org/draft-07/schema#",
    "$id":         "urn:schema:argparse",
    "title":       "argparse",
    "description": "specifies an command-line argument tree",
    "type":        "object",

    "properties": {
        "description": { "type": "string"                      },
        "prefixes":    { "type": "string", "default": "-"      },
        "args":        { "$ref": "#/definitions/argsContainer" }
    },
    
    "required":    [ "args" ],
    "definitions": {
        "arg": {
            "$comment":             "represents a single argument; that is, a key inside 'args'",
            "type":                 "object",
            "additionalProperties": False,
            
            "properties": {
                "type":        { "type": "string", "enum": [ "command", "int", "bool", "str", "float" ] },
                "description": {
                    "$comment": "enforces rule (4)",

                    "type":      "string",
                    "minLength": 1
                },
                "flag":        {
                    "$comment": "enforces rule (4)",
                    
                    "type":     "string",
                    "minLength": 1
                },
                "required":    { "type": "boolean"                                    },
                "choices":     { "type": "array", "minItems": 1                       },
                "action":      { "type": "string", "default": "store", "minLength": 1 },
                "default":     {                                                      },

                "args": {
                    "$comment": "represents the arguments and sub-commands of the current command",
                    "$ref":     "#/definitions/argsContainer"
                }
            },

            "required": [ "type", "description" ],
            "allOf": [
                {
                    "$comment": "enforces rules (1) and (2)",

                    "if":   { "properties": { "type":    { "const": "command"                    } } },
                    "then": { "properties": { "choices": { "not":   {} } }, "required": [ "args" ]   },
                    "else": { "properties": { "args":    { "not":   {} } }                           }
                },
                {
                    "$comment": "enforces rule (3)",

                    "allOf": [
                        {
                            "if":   { "properties": { "type":    { "const": "int"                 } } },
                            "then": { 
                                "allOf": [
                                    { "properties": { "choices": { "items": { "type": "integer" } } } },
                                    { "properties": { "default": { "type": [ "integer", "null" ]  } } }
                                ]
                            }
                        },
                        {
                            "if":   { "properties": { "type":    { "const": "float"               } } },
                            "then": { 
                                "allOf": [
                                    { "properties": { "choices": { "items": { "type": "number"  } } } },
                                    { "properties": { "default": { "type": [ "number", "null" ]   } } }
                                ]
                            }
                        },
                        {
                            "if":   { "properties": { "type":    { "const": "str"                 } } },
                            "then": { 
                                "allOf": [
                                    { "properties": { "choices": { "items": { "type": "string" }  } } },
                                    { "properties": { "default": { "type": [ "string", "null" ]   } } }
                                ]
                            }
                        },
                        {
                            "if":   { "properties": { "type":    { "const": "bool"                } } },
                            "then": { 
                                "allOf": [
                                    { "properties": { "choices": { "items": { "type": "boolean" } } } },
                                    { "properties": { "default": { "type": [ "boolean", "null" ]  } } }
                                ]
                            }
                        }
                    ]
                },
                {
                    "$comment": "enforces rule (6)",

                    "if":   { "properties": { "type": { "const": "command" } } },
                    "then": {
                        "allOf": [
                            { "properties": { "flag":     { "not": {} } } },
                            { "properties": { "required": { "not": {} } } },
                            { "properties": { "action":   { "not": {} } } },
                            { "properties": { "default":  { "not": {} } } }
                        ]
                    }
                },
                {
                    "$comment": "enforce rule (7)",

                    "if": {
                        "allOf": [
                            { "not": { "required": [ "flag" ]                           } },
                            { "properties": { "type": { "not": { "const": "command" } } } }
                        ]
                    },
                    "then": {
                        "allOf": [
                            { "required":   [ "required" ]                    },
                            { "properties": { "required": { "const": True } } }
                        ]
                    }
                },
                {
                    "$comment": "enforce rule (8)",

                    "if":   {
                        "required":   [ "required" ],
                        "properties": { "required": { "const": True } }
                    },
                    "then": { "properties": { "default": { "not": {} } } }
                }
            ]
        },
        "argsContainer": {
            "$comment":             "represents a container for nodes",
            "type":                 "object",
            "additionalProperties": False,
            
            "patternProperties": {
                "^[a-zA-Z_](?:[a-zA-Z0-9_\\-]*)$": {
                    "$comment": "represents an argument or sub-command",
                    "$ref":     "#/definitions/arg"
                }
            }
        }
    }
}


def ParseCommandLine(argCfg: str | dict[str, any]) -> tuple[ArgumentParser, Namespace] | None:
    def int_ProcessJSON(
            parser: ArgumentParser | None = None,
            obj:    dict[str, any]        = None,
            isTop:  bool                  = False,
            pre:    str | None            = '-',
            idx:    int                   = 0
    ) -> ArgumentParser | None:
        # If we are at the top-level, get 'prog' and 'description', then instantiate parser.
        if isTop:
            desc: str = obj.get('description') or None
            pre:  str = obj.get('prefixes')    or None

            parser = ArgumentParser(description=desc, prefix_chars=pre)

        # Scan arguments.
        argsObj:    dict[str, any] = obj.get('args', dict())
        subParsers: any            = None
        for k, v in argsObj.items():
            if v.get('type') == 'command':
                # Found sub-command. Add subparsers to current parser, then add the parser to the parent (i.e.,
                # current) parser.
                if not subParsers:
                    subParsers = parser.add_subparsers(title=f'cmd{idx}', required=True)
                newParser = subParsers.add_parser(k, help=v.get('description'))

                # Now process the sub-command. It should have similar structure like top-level command.
                int_ProcessJSON(newParser, v, False, pre, idx + 1)
            else:
                GL_TYPES_MAP: dict[str, type] = {
                    "int":    int,
                    "float":  float,
                    "string": str,
                    "bool":   bool
                }
                
                # If it is not a sub-command, we simply add the argument to the current parser.
                nameOrFlags: list[str] = [f'{pre * 2}{k}']
                if 'flag' in v:
                    nameOrFlags.insert(0, f'{pre}{v.get("flag")}')

                isReq: bool = v.get('required', False)
                parser.add_argument(
                    *nameOrFlags,
                    required=isReq,
                    type=GL_TYPES_MAP.get(v.get('type', 'string')),
                    action=v.get('action'),
                    help=v.get('description'),
                    default=v.get('default'),
                    choices=v.get('choices')
                )
    
        return parser

    # Get instance as python dict.
    instObj: dict[str, any] = None
    if isinstance(argCfg, str):
        try:
            with open(argCfg, 'r') as file:
                # Load JSON.
                try:
                    instObj = load(file)
                except JSONDecodeError as decErr:
                    print(f'kigen: error: Could not parse file "{argCfg}".')
                    print(f'kigen: error: Parsing failed with error: {decErr}')

                    return None
        except FileNotFoundError:
            print(f'kigen: error: Could not open file "{argCfg}".')

            return None
    elif isinstance(argCfg, dict):
        instObj = argCfg
    else:
        print('kigen: error: Command-line argument specification must be a python dict or a path to a file.')

        return None
        
    # Validate against schema.
    try:
        validate(instObj, GL_SCHEMA_ARGSJSON)
    except ValidationError as valErr:
        print(f'kigen: error: Validation of command-line argument specification against schema failed.')
        print(f'kigen: error: Validation failed with error: {valErr}')
        
        return None
        
    # Finally parse the arguments.
    parser  = int_ProcessJSON(None, instObj, True, None, 0)
    argsArr = parser.parse_args()
    # All good.
    return parser, argsArr
 


