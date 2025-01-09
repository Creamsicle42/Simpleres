# A simple packing script 
import io
import struct

files = ["test_file_1.txt", "test_file_2.txt"]

id_text = ""
id_data = {}

for fileid in files:
    id_data[fileid] = {
        "start": len(id_text),
        "len": len(fileid)
    }
    print(len(id_text))
    id_text += fileid

id_padding = len(id_text) % 4
for _ in range(id_padding):
    id_text += b'\00'.decode('utf-8')

with open("test_pack.smr", "wb") as f:
    f.write(b'smpr') # File intro
    f.write(b'\00\01') # File verson
    f.write(struct.pack('>H', len(files))) # Res count
    f.write(struct.pack('>I', len(id_text))) # ID length
    f.write(str.encode(id_text)) # ID Section
    for fileid in files: # Resource header first pass
        f.write(struct.pack('>I', id_data[fileid]["start"])) # ID start
        f.write(struct.pack('>H', id_data[fileid]["len"])) # ID Length
        f.write(b'\00\01') # Uncompressed flag
        id_data[fileid]["dat_start"] = f.tell()
        f.write(b'\00\00\00\00') # Write dummy data start
        id_data[fileid]["dat_length"] = f.tell()
        f.write(b'\00\00\00\00') # Write dummy compressed length
        f.write(b'\00\00\00\00') # Write dummy uncompressed length
    for fileid in files:
        with open(fileid, "rb") as subf:
            fstart = f.tell()
            f.write(subf.read())
            fend = f.tell()
            f.seek(id_data[fileid]["dat_start"])
            f.write(struct.pack('>I', fstart))
            f.seek(id_data[fileid]["dat_length"])
            f.write(struct.pack('>I', fend - fstart))
            f.write(struct.pack('>I', fend - fstart))
            f.seek(-1, 2)
    f.close()
