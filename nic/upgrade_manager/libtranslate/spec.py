import json

# Loads and parses the json spec file find in "filename"
def load_spec(filename):
    with open(filename, 'r') as f:
        return json.loads(f.read())