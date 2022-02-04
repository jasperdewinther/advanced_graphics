

minimum = 999999.0
f = open("dragon.obj", "r")
for x in f:
    if x[0] == "v" and x[1] == " ":
        text = x.split()
        minimum = min(minimum, float(text[2]))
print(minimum)
