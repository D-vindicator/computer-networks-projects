import random
import time
def mergeSort(alist):
    print("Splitting ",alist)
    if len(alist)>1:
        mid = len(alist)//2
        lefthalf = alist[:mid]
        righthalf = alist[mid:]

        mergeSort(lefthalf)
        mergeSort(righthalf)

        i=0
        j=0
        k=0
        while i < len(lefthalf) and j < len(righthalf):
            if lefthalf[i] < righthalf[j]:
                alist[k]=lefthalf[i]
                i=i+1
            else:
                alist[k]=righthalf[j]
                j=j+1
            k=k+1

        while i < len(lefthalf):
            alist[k]=lefthalf[i]
            i=i+1
            k=k+1

        while j < len(righthalf):
            alist[k]=righthalf[j]
            j=j+1
            k=k+1
    print("Merging ",alist)

def randomArray(array):
    for i in range(1,1001):
        array.append(random.randrange(1,10000000))
    print("random array generated!")
    print(array)
    #print(id(array))

#alist = [1, 5, 3, 4, 7, 6, 8, 2]
alist = []
randomArray(alist)
start_time = time.time()
mergeSort(alist)
print(alist)
print("sorting time " , time.time() - start_time)

