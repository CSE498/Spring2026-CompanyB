world traffic;

let square_walker : car {
    init : {
    };
    turn : {
        let a : int = 0;
        if (0) {
            move(right);
        } else {
            move(left);
        }
        a = a + 1;
    };
};
