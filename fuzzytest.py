import os
import subprocess
from nilsimsa import Nilsimsa
import tlsh


hashes = [
    [],  # nilsimsa
    [],  # tlsh
    []   # ssdeep
]

compare_results = [
    [],  # nilsimsa
    [],  # tlsh
    []   # ssdeep
]

for filename in os.listdir('.'):
    if filename.startswith("prog"):
        with open(filename, "rb") as file:
            file_data = file.read()
            
            # nilsimsa
            hashes[0].append(Nilsimsa(file_data))
            
            # tlsh
            hashes[1].append((filename, tlsh.hash(file_data)))
            
            # ssdeep
            output = subprocess.Popen(["ssdeep.exe", filename], stdout=subprocess.PIPE).communicate()[0]
            hashes[2].append((filename, output))
            #hashes[2].append((filename, str(output.splitlines()[2]).split("\'", 1)[1].split(",", 1)[0]))

print("")
print("nilsimsa (different 0 - 128 similar)")
for e in hashes[0]:
    print(str(e.hexdigest()))
    compare_results[0].append(hashes[0][0].compare(e.hexdigest(), True))

print("")
print("tlsh (different ? - 0 similar)")
for e in hashes[1]:
    print(str(e))
    compare_results[1].append(tlsh.diff(hashes[1][0][1], e[1]))

print("")
print("ssdeep (different 0 - 100 similar)")
with open("tmp", "wb") as file:
    file.write(hashes[2][0][1])
for e in hashes[2]:
    print(str(e))
    output = subprocess.Popen(["ssdeep.exe", "-a", "-m", "tmp", e[0]], stdout=subprocess.PIPE).communicate()[0]
    compare_results[2].append(int(str(output).split("(", 1)[1].split(")", 1)[0]))

print("")
print(str(compare_results))



