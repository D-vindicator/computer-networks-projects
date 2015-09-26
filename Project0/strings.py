list_of_string = ["computer networks",\
 "computer", "network", "net", "columbia university",\
  "columbia", "united states of america", "united", "united states" ]
remove_list = [x for y in list_of_string for x in list_of_string if (x in y and x != y)]
result_list = [x for x in list_of_string if x not in remove_list]
print(result_list)
# for x in list_of_string:
# 	print("x: ",x)
# 	for y in list_of_string:
# 		print("y: ",y)
# 		if y in x and y!=x:
# 			list_of_string.remove(y)
# 			print("removing ",y)
# print(list_of_string)


