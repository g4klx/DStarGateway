import json
import csv
import sys
import os

def convert_json_to_csv():
    # Set default filenames
    input_file = 'dstar_hosts.json'
    output_file = 'dstar_hosts.csv'

    # Override defaults if command-line arguments are provided
    if len(sys.argv) > 1:
        input_file = sys.argv[1]
    if len(sys.argv) > 2:
        output_file = sys.argv[2]

    # Check if input file exists
    if not os.path.exists(input_file):
        print(f"Error: Input file '{input_file}' not found.")
        sys.exit(1)

    header = ['#NAME', 'TYPE', 'ADDRESS', 'PORT']

    try:
        # Load JSON data from file
        with open(input_file, 'r', encoding='utf-8') as f:
            data = json.load(f)

        # Open CSV for writing
        with open(output_file, mode='w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(header)
            
            # Extract reflectors list
            reflectors = data.get('reflectors', [])
            
            for item in reflectors:
                row = [
                    item.get('name', ''),
                    item.get('reflector_type', ''),
                    item.get('ipv4', ''),
                    item.get('port', '')
                ]
                writer.writerow(row)
                
        print(f"Success: Processed '{input_file}' and created '{output_file}'")

    except json.JSONDecodeError as e:
        print(f"Error: Failed to parse JSON: {e}")
    except IOError as e:
        print(f"I/O Error: {e}")

if __name__ == "__main__":
    convert_json_to_csv()