(set-info :smt-lib-version 2.6)
(set-logic QF_LIA)
(set-info :category "crafted")
(set-info :status sat)
(declare-fun x_0 () Int)
(declare-fun x_1 () Int)
(declare-fun x_2 () Int)
(declare-fun x_3 () Int)
(declare-fun x_4 () Int)
(declare-fun x_5 () Int)
(declare-fun x_6 () Int)
(assert (>= x_0 0))
(assert (>= x_1 0))
(assert (>= x_2 0))
(assert (>= x_3 0))
(assert (>= x_4 0))
(assert (>= x_5 0))
(assert (>= x_6 0))
(assert (<= (+ (* (- 57) x_0) (* 2 x_1) (* 2 x_2) (* 2 x_3) (* 2 x_4) (* 2 x_5) (* 2 x_6)) 0))
(assert (<= (+ (* 3 x_0) (* (- 56) x_1) (* 3 x_2) (* 3 x_3) (* 3 x_4) (* 3 x_5) (* 3 x_6)) 0))
(assert (<= (+ (* 5 x_0) (* 5 x_1) (* (- 54) x_2) (* 5 x_3) (* 5 x_4) (* 5 x_5) (* 5 x_6)) 0))
(assert (<= (+ (* 7 x_0) (* 7 x_1) (* 7 x_2) (* (- 52) x_3) (* 7 x_4) (* 7 x_5) (* 7 x_6)) 0))
(assert (<= (+ (* 11 x_0) (* 11 x_1) (* 11 x_2) (* 11 x_3) (* (- 48) x_4) (* 11 x_5) (* 11 x_6)) 0))
(assert (<= (+ (* 13 x_0) (* 13 x_1) (* 13 x_2) (* 13 x_3) (* 13 x_4) (* (- 46) x_5) (* 13 x_6)) 0))
(assert (<= (+ (* 17 x_0) (* 17 x_1) (* 17 x_2) (* 17 x_3) (* 17 x_4) (* 17 x_5) (* (- 42) x_6)) 0))
(assert (>= (+ x_0 x_1 x_2 x_3 x_4 x_5 x_6) 1))
(check-sat)
(exit)
