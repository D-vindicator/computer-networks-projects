import threading
import queue
import random
import time

def mergeSort(inputlist):
	start_time = time.time()
	if len(inputlist) > 1:
		mid = len(inputlist)//2
		lefthalf = inputlist[:mid]
		righthalf = inputlist[mid:]
		
		merge_thread1 = threading.Thread(target = mergeSort, args= (lefthalf,))
		merge_thread1.start()
		merge_thread2 = threading.Thread(target = mergeSort, args= (righthalf,))
		merge_thread2.start()
		merge_thread1.join()
		merge_thread2.join()

		i = 0
		j = 0
		k = 0
		while i < len(lefthalf) and j < len(righthalf):
			if lefthalf[i] < righthalf[j]:
				inputlist[k] = lefthalf[i]
				i =i + 1
			else:
				inputlist[k] = righthalf[j]
				j =j + 1
			k = k + 1

		while i < len(lefthalf):
			inputlist[k] = lefthalf[i]
			i = i + 1
			k = k + 1

		while j < len(righthalf):
			inputlist[k] = righthalf[j]
			j = j + 1
			k = k + 1
	print("list length: ", len(inputlist)," time elipsed: ", time.time()-start_time)

def randomArray(array):
	for i in range(1,1001):
		array.append(random.randrange(1,10000000))
	print("random array generated!")
	#print(array)
	#print(id(array))

def main():
	#array = [2,1,4,3,6,8,7,5]
	array = []
	randomArray(array)
	#print(array)
	#print(id(array))
	start_time = time.time()
	mergeSort(array)
	#print(array)
	print("sorting time: ", time.time() - start_time)
	

if __name__ == '__main__':
	main()
