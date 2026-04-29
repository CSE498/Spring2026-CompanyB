// Car definitions for the traffic demo in (web_main.cpp).

world traffic;

// def_idx 0: Vertical pacer.
// Tries down/up first; if last turn didn't change y, falls back to right/left
// alternating so the agent can reroute around blocked cells/lights.
let v_pacer : car {
    init : {
        let a : int = 0;
        let last_y : int = -1;
        let side : int = 0;            // 0 = right, 1 = left
        __spawn__ = make_point(306, 142);
        // Destination is set so DataLog counts this car under driving_count.
        __destination__ = make_point(306, 200);
    };
    turn : {
        let y : int = get_y(__position__);

        if (y == last_y) {
            // Last turn's vertical move didn't take -- try sideways.
            if (side == 0) move(right);
            else            move(left);
            side = 1 - side;
        } else if ((a % 20) < 10) {
            move(down);
        } else {
            move(up);
        }

        last_y = y;
        a = a + 1;
    };
};

// def_idx 1: Destination walker.
// Primary of greedy axis-closing toward destination.
// Fallback if last turn didn't change position: rotate through up, right,
// left, right until one sticks; reset on any successful move.
let road_walker : car {
    init : {
        __spawn__ = make_point(16, 389);
        __destination__ = make_point(306, 389);
        let last_x : int = -1;
        let last_y : int = -1;
        let stuck_step : int = 0;
    };
    turn : {
        let x   : int = get_x(__position__);
        let y   : int = get_y(__position__);
        let d_x : int = get_x(__destination__);
        let d_y : int = get_y(__destination__);

        // "stuck" if and only if both x and y unchanged since last turn.
        if (x == last_x) {
            if (y == last_y) {
                if      ((stuck_step % 4) == 0) move(up);
                else if ((stuck_step % 4) == 1) move(right);
                else if ((stuck_step % 4) == 2) move(left);
                else                            move(right);
                stuck_step = stuck_step + 1;
            } else {
                stuck_step = 0;
                if      (y < d_y) move(down);
                else if (y > d_y) move(up);
            }
        } else {
            stuck_step = 0;
            if      (x < d_x) move(right);
            else if (x > d_x) move(left);
            else if (y < d_y) move(down);
            else if (y > d_y) move(up);
        }

        last_x = x;
        last_y = y;
    };
};
