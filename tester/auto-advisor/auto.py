File = open("failed-random-1.txt") 

name = []
credit = []
pre = []

while True:
    text = file.readline()  # 只读取一行内容
 
    # 判断是否读取到内容
    if not text:
        break
 
    # 每读取一行的末尾已经有了一个 `\n`
    line_data = test.split('|')

    name.append(line_data[0])
    credit.append(int(line_data[1]))

    pre_set = line_data[2].split(';')
    pp_set = []
    for pres in pre_set:
        pre


 
 
file.close()
