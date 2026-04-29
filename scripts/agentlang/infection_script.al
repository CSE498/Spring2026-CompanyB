// Pacer definitions for the infection demo in (web_main.cpp)

// def_idx 0: Horizontal pacer
// Walks right for span cells, then left for span cells, then repeats.

world infection;

let h_pacer : student {
    init : {
        let self.step_idx : int = 0;
        let const self.span : int = 12;
    };
    turn : {
        // Phase 0: stepping right; phase 1: stepping left.
        let phase : int = (self.step_idx / self.span) % 2;

        if (phase == 0) move(right);
        else            move(left);

        self.step_idx = self.step_idx + 1;
    };
};


// def_idx 1: Vertical pacer
// Walks down for span cells, then up for span cells, then repeats.

let v_pacer : student {
    init : {
        let self.step_idx : int = 0;
        let const self.span : int = 10;
    };
    turn : {
        let phase : int = (self.step_idx / self.span) % 2;

        if (phase == 0) move(down);
        else            move(up);

        self.step_idx = self.step_idx + 1;
    };
};


// def_idx 2: Square walker
// Cycles right -> up -> left -> down so the agent traces a rectangle of side
// span.

let square_walker : student {
    init : {
        let self.step_idx : int = 0;
        let const self.span : int = 8;
    };
    turn : {
        let leg : int = (self.step_idx / self.span) % 4;

        if      (leg == 0) move(right);
        else if (leg == 1) move(up);
        else if (leg == 2) move(left);
        else               move(down);

        self.step_idx = self.step_idx + 1;
    };
};


// def_idx 3: Stationary observer
// Emits no movement steps. Why not!

let stander : student {
    init : ;
    turn : ;
};
