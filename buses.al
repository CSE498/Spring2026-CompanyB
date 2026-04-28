world traffic;

let square_walker : car {
    init : {
        let a : int = 0;
        __spawn__ = make_point(30, 11);
    };
    turn : {
        if (a < 23) {
            move(right);
        } else if (a < 26) {
            move(down);
        } else if (a < 36) {
            move(right);
        } else if (a < 49) {
            move(down);
        } else if (a < 68) {
            move(left);
        } else if (a < 78) {
            move(up);
        } else if (a < 93) {
            move(left);
        } else if (a < 98) {
            move(up);
        } else if (a < 100) {
            move(right);
        } else {
            a = 0;
        }
        a = a + 1;
    };
};