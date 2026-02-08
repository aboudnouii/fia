power(X,0,1).
power(X,Y,Z):- Y1 is Y -1,power(X,Y1,Z1), Z is Z1 * X.
un(X,0):- X is 2.
un(X,N):- N1 is N - 1,un(Z,N1),X is  2*Z + 3.
:- use_module(library(clpfd)).
sudoku(Rows) :-
    length(Rows, 9), maplist(same_length(Rows), Rows),
    append(Rows, Vs), Vs ins 1..9,
    maplist(all_distinct, Rows),
    transpose(Rows, Columns),
    maplist(all_distinct, Columns),
    Rows = [A,B,C,D,E,F,G,H,I],
    blocks(A,B,C), blocks(D,E,F), blocks(G,H,I),
    maplist(label, Rows).
blocks([], [], []).
blocks([A,B,C|Bs1], [D,E,F|Bs2], [G,H,I|Bs3]) :-
    all_distinct([A,B,C,D,E,F,G,H,I]),
    blocks(Bs1, Bs2, Bs3).
