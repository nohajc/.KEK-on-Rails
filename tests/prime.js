const n = 100, sqrtn = 10;
//var i, j, isprime[2..n+1*3], prime[0..n];

isprime = []
prime = []

for(i = 2; i <= n; ++i){
	isprime[i] = 1;
}

/*
 *****************************
 * The sieve of Eratosthenes *
 *****************************
 */
for(i = 2; i <= sqrtn; ++i){
	if(isprime[i] == 1){
		j = i * i;
		while(j <= n){
			isprime[j] = 0;
			j = j + i;
		}
	}

	j = 0;
	for(i = 2; i <= n; ++i){
		if(isprime[i] == 1){
			prime[j] = i;
			j = j + 1;
		}
	}

	for(i = 0; i < j; ++i) write prime[i];
}
