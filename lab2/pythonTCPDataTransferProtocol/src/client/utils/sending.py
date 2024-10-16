"""here some funcs related to preparation data
    for sending it trough TCP,
    for example, parse objects to json"""

import json
from src.protocol.messages import *

def convert_to_json(message):
    return json.dumps(message)