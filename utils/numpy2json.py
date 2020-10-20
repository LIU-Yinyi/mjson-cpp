import json
import numpy as np


def numpy2json(data, jsonfile):
    assert type(data) == dict
    for k in data.keys():
        if type(data[k]) != list:
            if type(data[k]) == np.ndarray:
                    data[k] = data[k].tolist()
    with open(jsonfile, 'w') as f:
        str = json.dump(data, f, indent=4)

