const n = 100, sqrtn = 10;
var i, j, isprime[2..n+1*3], prime[0..n];

begin
	for i := 2 to n do isprime[i] := 1;

	{The sieve of Eratosthenes}
	for i := 2 to sqrtn do
		if isprime[i] = 1 then begin
			j := i * i;
			while j <= n do begin
				isprime[j] := 0;
				j := j + i;
			end
		end;
		
	j := 0;
	for i := 2 to n do 
		if isprime[i] = 1 then begin 
			prime[j] := i;
			j := j + 1;
		end;

	for i := 0 to j - 1 do write prime[i];
end
