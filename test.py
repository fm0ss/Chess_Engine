n = int(input("Please input the amount of numbers you are going to input: "))
nums = [int(input("Input the next number: ")) for _ in range(n)]

freq = {}

for i in nums:
    if i not in freq.keys():
        freq[i] = 1
    else:
        freq[i] += 1

count = 0
max = 0
max_num = 0
for i in freq.keys():
    if freq[i] > max:
        max = freq[i]
        max_num = i
        count = 1
    elif freq[i] == max:
        count += 1

if count == 1:
    print("{} (the modal digit) was entered {} times".format(max_num,max))
elif count == 0:
    print("No data was entered")
else:
    print("Data was multimodal")

