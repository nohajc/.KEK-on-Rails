(define (nil? a) (eq? a #nil))
(define (len a) (if (nil? a) 0 (+ 1 (len (cdr a)))))
(define (append a b) (if (nil? a) b (cons (car a) (append (cdr a) b))))
