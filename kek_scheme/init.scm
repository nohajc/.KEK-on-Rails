(define (cadr l) (car (cdr l)))
(define (cddr l) (cdr (cdr l)))
(define (null? a) (eq? a null))
(define (zero? n) (= n 0))
(define (eof-object? a) (null? a))
(define (length a) (if (null? a) 0 (+ 1 (length (cdr a)))))
(define (append a b) (if (null? a) b (cons (car a) (append (cdr a) b))))

(define (filter pred lst)
  (if (null? lst)
	 null
	 (if (pred (car lst))
		(cons (car lst) (filter pred (cdr lst)))
		(filter pred (cdr lst)))))

(define (map fn lst)
  (if (null? lst)
	 null
	 (cons (fn (car lst)) (map fn (cdr lst)))))

(define (member v lst)
  (if (null? lst)
	 #f
	 (or (equal? (car lst) v)
		  (member v (cdr lst)))))

(define (uniq_r lst added)
  (if (null? lst)
	 null
	 (if (member (car lst) added)
		(uniq_r (cdr lst) added)
		(cons (car lst) (uniq_r (cdr lst) (cons (car lst) added))))))

(define (remove-duplicates lst)
  (uniq_r lst null))

(define (fact_t n a) (if (= n 1) a (fact_t (- n 1) (* n a))))
(define (fact n) (fact_t n 1))
