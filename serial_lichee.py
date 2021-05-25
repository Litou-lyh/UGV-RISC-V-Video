from serial import Serial

ser = Serial("com4", 115200, timeout=1)

def ReadUntill(word, stopwhenendl=True):
    rec = []
    flag = False
    while True:
        try:
            c = ser.read(1).decode()
        except:
            continue
        rec.append(c)
        if "".join(rec).find(word) != -1:
            flag = True
            break
        if stopwhenendl and c == "\n" and len(rec) > 1:
            flag = False
            break
    return "".join(rec), flag


def ReadUntillProcess(word):
    while True:
        ret, flag = ReadUntill(word)
        print(ret, end="")
        if flag:
            break

# ReadUntillProcess("debian login:")
# ser.write("root\n".encode())
# ReadUntillProcess("Password:")
# ser.write("root\n".encode())
ser.write("\n".encode())
while True:
    ReadUntillProcess("root@debian:")
    ReadUntillProcess("#")
    cmd = input()
    ser.write((cmd+"\n").encode())
    if cmd == "ssend":
        ReadUntill("_+_SOF_+_", False)
        f = open("x.txt", "w")
        while True:
            data, flag = ReadUntill("_+_EOF_+_")
            if not flag:
                f.write(data)
            else:
                break
        f.close()
        # os.system("base64 -d x.txt >x.jpg")
        # os.systme("fbi x.jpg")
