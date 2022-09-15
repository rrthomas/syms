#!/usr/bin/env -S guile -s
!#
(define prog-name "syms")

(use-modules
 (ice-9 exceptions)
 (ice-9 getopt-long)
 (ice-9 format)
 (ice-9 rdelim)
 (ice-9 regex)
)

;; Command-line options
(define option-spec
  '((version (single-char #\V) (value #f))
    (help    (single-char #\h) (value #f))
    (symbol  (single-char #\s) (value #t))
))

;; Parse command-line args
(setlocale LC_ALL "")
(define options (getopt-long (command-line) option-spec))
(define symbol-pat (option-ref options 'symbol "([[:alpha:]]+)"))

(define* (message-and-exit s #:optional [code 0])
  (display s)
  (exit code))
(define (die s)
  (message-and-exit (format #f "~a: ~a" prog-name s) 1))

(if (option-ref options 'version #f)
    (message-and-exit (format #t "~a 0.9 (14 Sep 2022) by Reuben Thomas <rrt@sc3d.org>\n" prog-name)))
(if (option-ref options 'help #f)
    (message-and-exit (format #t "Usage: ~a ~a\n" prog-name "[OPTION]... [FILE]...

List symbols in input.

  -s, --symbol=REGEXP      symbols are given by REGEX
  -h, --help               display this help, then exit
  -V, --version            display version information, then exit

The default symbol type is words (-s \"([[:alpha:]]+)\"); other useful settings
include:

  non-white-space characters: -s \"[^[:space:]]+\"
  alphanumerics and underscores: -s \"[[:alnum:]_]+\"
  XML tags: -s \"<([a-zA-Z_:][a-zA-Z_:.0-9-]*)[[:space:]>]\"")))

;; Compile symbol-matching regexp
(define rx
  (with-exception-handler
      (lambda (exn) (die (format #f "~a\n" (exception-message exn))))
    (lambda () (make-regexp symbol-pat))
    #:unwind? #t))

;; Process input
(define (find-and-print-matches port)
  (while (not (eof-object? (peek-char port)))
    (fold-matches rx (read-line port) #f
                  (lambda (m count)
                    (display (format #f "~a\n" (match:substring m)))))))

(for-each
 (lambda (arg) (call-with-input-file arg find-and-print-matches))
 (option-ref options '() #f))
