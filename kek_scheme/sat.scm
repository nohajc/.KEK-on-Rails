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
; parsed: ((a b c) ((! a) (! c)) (b))

(define (read-liter tok)
  (if (string=? (car tok) "!")
	 (cons (list '! (cadr tok)) (cddr tok))
	 tok))

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
		  (cons (cons (car cl-t) (car fr-t)) (cdr fr-t))))
	 (let ((lit-t (read-liter tok)))
		(let ((fr-t (read-form-rest (cdr lit-t))))
        (cons (cons (list (car lit-t)) (car fr-t)) (cdr fr-t))))))

(define (read-formula tok)
  (car (read-form tok)))

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

(define (read-input f)
  (let ((ln (read-line f)))
	 (unless (eof-object? ln)
		(let ((tokens
        (string-split
          (sep-keywords ln '("(" ")" "&" "|" "!")))))
		  (print (read-formula tokens))
        (newline)
        (read-input f)))))

;------------------------------------------------------------
(define argv (current-command-line-arguments))
(when (zero? (vector-length argv))
  (exit 1))
(define in_fname (vector-ref argv 0))

(define in_f (open-input-file in_fname))

(read-input in_f)

(close-input-port in_f)

