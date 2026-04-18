world traffic;

let square_walker : car {
    init : {
    };
    turn : {
        let a : int = 0;
        if (a % 2) {
            move(right);
        } else {
            move(left);
        }
        a = a + 1;
    };
};
