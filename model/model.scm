#lang racket

(define receptor-shapes (list 'square 'triangle 'donut 'guitar))
(define num-virus-receptors 5)
(define num-host-receptors 5)
(define virus-mutation-rate 10)
(define virus-threshold 50)
(define virus-popsize 10)
(define host-popsize 100)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (choose l) (list-ref l (random (length l))))

(define (generate-receptors size)
  (build-list
   size
   (lambda (_)
     (choose receptor-shapes))))

(define (mutate-receptors receptors rate)
  (map
   (lambda (r)
     (if (< (random 10000) (* rate 100))
         (choose receptor-shapes)
         r))
   receptors))

(define (receptor-count receptor receptors)
  (foldl
   (lambda (r v)
     (if (eq? r receptor) (+ v 1) v))
   0 receptors))

(define (receptor-histogram receptors)
  (map
   (lambda (shape)
     (receptor-count shape receptors))
   receptor-shapes))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (make-virus receptors fitness) (list receptors fitness))
(define (virus-receptors v) (list-ref v 0))
(define (virus-fitness v) (list-ref v 1))

(define (build-virus)
  (make-virus (generate-receptors num-virus-receptors) 0))

(define (spawn-virus v rate)
  (make-virus
   (mutate-receptors (virus-receptors v) rate)
   0))

;; could do fitness via histograms
;; but this stochastically is what is happening
(define (virus-attempt-attach? virus host-receptors)
  (eq?
   (choose (virus-receptors virus))
   (choose host-receptors)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (viruses-repop v rate)
  (append v (build-list (- virus-popsize (length v))
                        (lambda (_)
                          (spawn-virus (choose v) rate)))))

(define (viruses-decimate v threshold)
  (cadr
   (foldl
    (lambda (virus r)
      (let ((index (car r))
            (l (cadr r)))
        (if (> index (* (/ threshold 100) (length v)))
            (list (+ index 1)
                  (cons virus l))
            (list (+ index 1)
                  l))))
    (list 0 '()) v)))

          
(define (viruses-sort v)
  (sort v (lambda (a b) (< (virus-fitness a) (virus-fitness b)))))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (make-world viruses hosts generations) (list viruses hosts generations))
(define (world-viruses w) (list-ref w 0))
(define (world-host-receptors w) (list-ref w 1))
(define (world-generations w) (list-ref w 2))

(define (build-world virus-popsize host-popsize)
  (make-world
   (build-list
    virus-popsize
    (lambda (_)
      (build-virus)))
   (build-list
    host-popsize
    (lambda (_) (list 'square 'square 'guitar 'donut 'donut)))
   0))

(define (world-calc-fitness virus w)
  (foldl
   (lambda (host-receptors r)
     (if (virus-attempt-attach? virus host-receptors)
         (+ r 1)
         r))
   0 (world-host-receptors w)))

(define (world-update-fitness w)
  (make-world
   (map
    (lambda (virus)
      (make-virus
       (virus-receptors virus)
       (world-calc-fitness virus w)))
    (world-viruses w))
   (world-host-receptors w)
   (world-generations w)))

(define (world-repopulate-viruses w)
  (make-world
   (viruses-repop
    (viruses-decimate    
     (viruses-sort (world-viruses w))
     virus-threshold)
    virus-mutation-rate)
   (world-host-receptors w)
   (+ (world-generations w) 1)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define w (build-world virus-popsize host-popsize))
(set! w (world-update-fitness w))

(define (update! n)
  (when (not (zero? n))
    (set! w (world-repopulate-viruses w))
    (set! w (world-update-fitness w))
    (display (car (world-viruses w)))(newline)
    (update! (- n 1))))
  
(update! 100)

