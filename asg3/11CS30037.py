import math
import cmath

def fft(a,x):
	n = len(a)
	if(n <= 0):
		print "\nERROR going to use FFT on list of size <=0"
		quit()
	A = [complex] * n 
	if (n==1):
		A[0] = a[0]
		return A
	if(x == 1):
		wn = complex(cmath.rect(1, 2*cmath.pi/n))
	else:
		wn = complex(1,0)/complex(cmath.rect(1, 2*cmath.pi/n))
	w = complex(1,0)
	n_2 = n/2
	a_even = [complex] * n_2 
	a_odd = [complex] * n_2
	for i in range(0,n_2):
		a_even[i] = a[i*2]
		a_odd[i] = a[i*2+1]
	A_even = fft(a_even,x)
	A_odd = fft(a_odd,x)
	for k in range(0,n_2):
		A[k] = A_even[k] + (A_odd[k] * w)
		A[k+n_2] = A_even[k] - (A_odd[k] * w)
		w = w * wn
	return A

def nextPowerofTwo(n):
	k = 1
	while(True):
		if(k>=n):
			return k
		k = k * 2

def max(a,b):
	if(a>=b):
		return a
	return b

def paddWithZero(a,N,paddingElt):
	n = len(a)
	if(n == 0):
		return
	eSize = N-n
	if(eSize == 0):
		return
	b = [type(a[0])] * eSize
	for i in range(0,eSize):
		b[i] = paddingElt
	c = a+b
	return c

def fftmult(a,b):
	n1 = len(a)
	n2 = len(b)
	a.reverse()
	b.reverse()
	n = max(n1,n2)
	n = nextPowerofTwo(n)
	x = paddWithZero(a,2*n,0)
	y = paddWithZero(b,2*n,0)
	A = fft(x,1)
	B = fft(y,1)
	C = [complex] * 2 * n
	for i in range(0,2*n):
		C[i] = A[i] * B[i]
	c = fft(C,0)
	d = [int] * 2 * n
	for i in range(0,2*n):
		d[i] = int(round(c[i].real/(2*n)))
	d.reverse()
	return d

def main():
	s = raw_input("Enter Digits separated by space :\n")
	a = [int(x) for x in s.split()]
	s = raw_input("Enter Digits separated by space :\n")
	b = [int(x) for x in s.split()]
	print "\na = ",a
	print "\nb = ",b
	c = fftmult(a,b)
	print "\noutput = \n",c
	return

main()
quit()
