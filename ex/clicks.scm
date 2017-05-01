(define (ps-values name scale) 
 (letrec
  ((set-values 
    (lambda (s tbl pos) 
     (if (< pos (vector-length s)) 
      (begin 
       (ps-tset tbl pos (vector-ref s pos)) 
       (set-values s tbl (+ pos 1))))))) 
  (set-values scale (ps-mkftbl "notes" (vector-length scale)) 0)))

(ps-values "notes" (vector 60 60 67 74 58 65 72 56 64))

(ps-copy "val" "val")
(ps-copy "s4" "decay")
(ps-mkvar "env")

(define (samphold val) (string-append " _"val" get dup 0 ne samphold "))

(ps-parse 0 "_val get 0.001 0.002 0.001 tenvx _env set")
;(ps-parse 0 
; (string-append 
;  "_env get" (samphold "val") "_notes tget mtof 0.3 1 7 _env get 1 * fm *"))

(ps-parse 0 
 (string-append
  "-0.2 0.2" (samphold "val") "800 * randh _env get * " ))

(ps-parse 0 (string-append "dup" (samphold "val") "600 * 0.8 diode bal "))
(ps-parse 0 (string-append (samphold "val") "_notes tget mtof _decay get 0.1 0.99 scale streson"))
(ps-init-sporthlet 0)
(ps-turnon 0 -1)
