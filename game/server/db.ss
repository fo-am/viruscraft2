;; Copyright (C) 2013 Dave Griffiths
;;
;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU Affero General Public License as
;; published by the Free Software Foundation, either version 3 of the
;; License, or (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU Affero General Public License for more details.
;;
;; You should have received a copy of the GNU Affero General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

#lang racket
(require (planet jaymccarthy/sqlite:5:1/sqlite))
(provide (all-defined-out))
(require "logger.ss")

(define (ms->frac ms)
  (modulo (inexact->exact (round ms)) 1000))

(define (timestamp-now)
  (let* ((ms (current-inexact-milliseconds))
         (t (seconds->date (inexact->exact (round (/ ms 1000))))))
    (string-append
     (number->string (date-year t)) "-"
     (substring (number->string (+ (date-month t) 100)) 1 3) "-"
     (substring (number->string (+ (date-day t) 100)) 1 3) " "
     (substring (number->string (+ (date-hour t) 100)) 1 3) ":"
     (substring (number->string (+ (date-minute t) 100)) 1 3) ":"
     (substring (number->string (+ (date-second t) 100)) 1 3) "."
     ;; get fractional second from milliseconds
     (substring (number->string (+ (ms->frac ms) 1000)) 1 4)
     )))

(define (setup db)
  (exec/ignore db "create table player ( id integer primary key autoincrement)")
  (exec/ignore db "create table game ( id integer primary key autoincrement, player_id integer, time_stamp varchar, stage integer, level integer, survived integer, duration real, mutations integer, hosts_spawned integer, viruses_spawned integer, infections integer, max_hosts integer, max_viruses integer, final_hosts integer, final_viruses integer, score real)")
  (exec/ignore db "create table player_name ( id integer primary key autoincrement, player_id integer, player_name text )")
  )

(define (insert-player db)
  (insert db "INSERT INTO player VALUES (NULL)"))

(define (insert-game db player_id)
  (insert
   db "INSERT INTO game VALUES (NULL, ?, ?, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)"
   player_id (timestamp-now)))

;; these are used to calculate the single score value
;; need to update these based on the client
;; if these change then all the scores are wrong anyway
;; so the db will need clearing
(define seconds-per-level 60)
(define levels-per-stage 5)

(define (calc-score duration level stage)
  (+ duration
	 ;; level starts at one
     (* (+ (- level 1)
		   ;; stage starts at zero
           (* stage levels-per-stage))
        seconds-per-level)))

(define (update-score db game_id stage level survived duration mutations hosts_spawned viruses_spawned infections max_hosts max_viruses final_hosts final_viruses)
  (exec/ignore
   db "update game set stage=?, level=?, survived=?, duration=?, mutations=?, hosts_spawned=?, viruses_spawned=?, infections=?, max_hosts=?, max_viruses=?, final_hosts=?, final_viruses=?, score=? where id = ?"
   stage level survived duration mutations hosts_spawned viruses_spawned infections max_hosts max_viruses final_hosts final_viruses
   (calc-score duration level stage)
   game_id))

(define (insert-player-name db player_id player_name)
  (insert db "insert into player_name VALUES (NULL, ?, ?)"
          player_id player_name ))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; get a list of all the scores
(define (get-player-name db player-id)
  (let* ((s (select db "select player_name from player_name where player_id = ?" player-id)))
    (if (null? s)
        "???"
        (vector-ref (cadr s) 0))))

;; get a list of all the scores
(define (get-game-scores db)
  (let* ((s (select db "select g.stage, g.level, g.duration from game as g
                        join player_name as n on g.player_id=n.player_id
                        where n.player_name != '???'
                        order by stage desc, level desc, duration desc")))
    (if (null? s)
        '()
        (map
         (lambda (i) (vector-ref i 0))
         (cdr s)))))

;; get the player name/scores ordered for the hiscores list
;; old version that orders the score manually
(define (hiscores-select_ db)
  (let ((r (select db "select n.player_name, g.stage, g.level, g.duration from game as g
                     join player_name as n on g.player_id=n.player_id        
                     where n.player_name !='???' and g.time_stamp > (select datetime('now', '-7 day'))
                     order by g.stage desc, g.level desc, g.duration desc limit 10")))
    (if (null? r) '() (cdr r))))

;; using the precached score value
(define (hiscores-select db)
  (let ((r (select db "select n.player_name, g.stage, g.level, g.duration from game as g
                     join player_name as n on g.player_id=n.player_id        
                     where n.player_name !='???' and g.time_stamp > (select datetime('now', '-7 day'))
                     order by g.score desc limit 10")))
    (if (null? r) '() (cdr r))))

;; using the precached score value to shorehorn this in one query
(define (get-game-rank db game-id)
  (let ((s (select db "select count(*) from game as g 
                       join player_name as n on g.player_id=n.player_id 
                       where n.player_name !='???' and 
                       g.score > (select score from game where id = ?) and 
                       g.time_stamp > (select datetime('now', '-7 day'))" game-id)))
    (if (null? s)
	1
	(+ (vector-ref (cadr s) 0) 1))))



