world traffic;

let square_walker : car {
    init : {
        let a : int = 0;
    };
    turn : {
        if (a % 2) {
            move(right);
        } else {
            move(left);
        }
        a = a + 1;
    };
};
