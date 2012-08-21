import subprocess

if __name__ == "__main__":
    echoer = subprocess.Popen(["echo"], stdin=subprocess.PIPE,
                              stdout=subprocess.PIPE)
    counter = subprocess.Popen(["wc"], stdin=echoer.stdout)
    for letter in "abcdefghi":
        print(letter)
        echoer.stdin.write(bytes(letter + "\n", "ascii"))
    
        
