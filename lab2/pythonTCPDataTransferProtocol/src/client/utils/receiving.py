"""here some funcs related to recieving data
    parsing json to objects etc."""

import json
from src.protocol.messages import *

def convert_json_to_obj(json_str):
    return json.loads(json_str)