world traffic;

let square_walker : car {
    init : {
        let have_moved : bool = false;
    };
    turn : {
        __destination__ = make_point(1,5);
    };
};