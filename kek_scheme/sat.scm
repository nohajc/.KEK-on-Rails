#! /usr/bin/env racket
#lang racket

; CNF Grammar
; form = ("(" clause ")", form_rest) | literal, form_rest
; formrest = ("&", form) | ""
; clause = literal, clause_rest
; clause_rest = ("|", clause) | ""
; literal = ("!", var) | var

; Example:
; input: (a | b | c) & (!a | !c) & b
; parsed: (lambda (a b c d) (and (or a b c) (or (not a) (not c)) (or b)))

(define (read-liter tok)
  (if (string=? (car tok) "!")
	 (cons (list 'not (string->symbol (cadr tok))) (cddr tok))
	 (cons (string->symbol (car tok)) (cdr tok))))

(define (read-clause-rest tok)
  (if (null? tok)
	 (list null)
	 (if (string=? (car tok) "|")
      (read-clause (cdr tok))
	   (cons null tok))))

(define (read-clause tok)
  (let ((lit-t (read-liter tok)))
	 (let ((clr-t (read-clause-rest (cdr lit-t))))
		(cons (cons (car lit-t) (car clr-t)) (cdr clr-t)))))

(define (read-form-rest tok)
  (if (null? tok)
	 (list null)
	 (if (string=? (car tok) "&")
	   (read-form (cdr tok))
      (cons null tok))))

(define (read-form tok)
  (if (string=? (car tok) "(")
	 (let ((cl-t (read-clause (cdr tok))))
		(let ((fr-t (read-form-rest (cddr cl-t))))
		  (cons (cons (cons 'or (car cl-t)) (car fr-t)) (cdr fr-t))))
	 (let ((lit-t (read-liter tok)))
		(let ((fr-t (read-form-rest (cdr lit-t))))
        (cons (cons (cons 'or (list (car lit-t))) (car fr-t)) (cdr fr-t))))))

(define (read-formula tok)
  (cons 'and (car (read-form tok))))

(define (sep-kw str kw)
  (string-replace str (car kw) (string-append " " (car kw) " ")))

(define (applyrec fn arg next acc)
  (if (null? arg)
	 acc
	 (applyrec fn (next arg) next (fn acc arg))))

(define (sep-keywords str kws)
  (applyrec
	 sep-kw
	 kws
	 (lambda (lst) (cdr lst))
	 str))

(define operators '("(" ")" "&" "|" "!"))

(define (var? a)
  (null? (filter (lambda (x) (string=? x a)) operators)))

(define (solve form varnum)
  (print form)
  (newline)
  (print varnum)
  (newline))

(define (read-input f)
  (let ((ln (read-line f)))
	 (unless (eof-object? ln)
		(let ((tokens
        (string-split
          (sep-keywords ln operators))))
		  (let ((formlst (read-formula tokens))
				  (vars (remove-duplicates (map string->symbol (filter var? tokens)))))
			 (solve (eval (list 'lambda vars formlst)) (length vars))
        (read-input f))))))

;------------------------------------------------------------
(define argv (current-command-line-arguments))
(when (zero? (vector-length argv))
  (exit 1))
(define in_fname (vector-ref argv 0))

(define in_f (open-input-file in_fname))

(read-input in_f)

(close-input-port in_f)

