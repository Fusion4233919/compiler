File = open("failed-random-1.txt") 

name = []
credit = []
pre = []

while True:
    text = file.readline()  # ֻ��ȡһ������
 
    # �ж��Ƿ��ȡ������
    if not text:
        break
 
    # ÿ��ȡһ�е�ĩβ�Ѿ�����һ�� `\n`
    line_data = test.split('|')

    name.append(line_data[0])
    credit.append(int(line_data[1]))

    pre_set = line_data[2].split(';')
    pp_set = []
    for pres in pre_set:
        pre


 
 
file.close()
