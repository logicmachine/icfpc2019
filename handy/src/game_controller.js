class Effect {
  constructor(p, b, a){
    this.coord  = p;
    this.before = b;
    this.after  = a;
  }
}

class HistoryRecord {
  constructor(cmd, position, effects, manips){
    this.command = cmd;
    this.position = position;
    this.effects = effects;
    this.manipulators = manips;
  }
}

class GameController {
  constructor(description, field_view){
    this.current_grid = description.grid;
    this.current_position = description.point;
    this.history = [];
    this.field_view = field_view;
    // Push initial wrapper state
    const initial_manips = [new Point(-1, 1), new Point(0, 1), new Point(1, 1)];
    const initial_reaches = this.compute_reaches(this.current_position, initial_manips);
    let initial_effects = [];
    initial_effects.push(new Effect(this.current_position, CELL_EMPTY, CELL_ROBOT));
    initial_reaches.forEach((r) => {
      initial_effects.push(new Effect(r, CELL_EMPTY, CELL_REACHING));
    });
    const record = new HistoryRecord('', this.current_position, initial_effects, initial_manips);
    this.push_record(record);
  }

  compute_reaches(pos, manips){
    const ccw = (ay, ax, by, bx, cy, cx) => {
      const dx = bx - ax, dy = by - ay;
      const ex = cx - ax, ey = cy - ay;
      const cross = dx * ey - dy * ex;
      if(cross == 0){ return 0; }
      return (cross > 0 ? 1 : -1);
    };
    const height = this.current_grid.length;
    const width  = this.current_grid[0].length;
    let reaches = []
    manips.forEach((offset) => {
      const r = pos.r + offset.r;
      const c = pos.c + offset.c;
      if(r < 0 || height <= r){ return; }
      if(c < 0 || width  <= c){ return; }
      if(this.current_grid[r][c] == CELL_OBSTACLE){ return; }
      const r_min = Math.min(pos.r, r);
      const r_max = Math.max(pos.r, r);
      const c_min = Math.min(pos.c, c);
      const c_max = Math.max(pos.c, c);
      let accept = true;
      for(let i = r_min; accept && i <= r_max; ++i){
        for(let j = c_min; accept && j <= c_max; ++j){
          if(this.current_grid[i][j] != CELL_OBSTACLE){ continue; }
          const ccws = [
            ccw(pos.r + 0.5, pos.c + 0.5, r + 0.5, c + 0.5, i + 0, j + 0),
            ccw(pos.r + 0.5, pos.c + 0.5, r + 0.5, c + 0.5, i + 0, j + 1),
            ccw(pos.r + 0.5, pos.c + 0.5, r + 0.5, c + 0.5, i + 1, j + 0),
            ccw(pos.r + 0.5, pos.c + 0.5, r + 0.5, c + 0.5, i + 1, j + 1)
          ];
          let acc = 0;
          ccws.forEach((c) => {
            if(acc == 0){
              acc = c;
            }else if(acc != c && c != 0){
              accept = false;
            }
          });
        }
      }
      if(accept){
        reaches.push(new Point(r, c));
      }
    });
    return reaches;
  }

  push_record(record){
    record.effects.forEach((e) => {
      this.current_grid[e.coord.r][e.coord.c] = e.after;
      this.field_view.update(e.coord, e.after);
    });
    this.history.push(record);
  }

  pop_record(){
    if(this.history.length <= 1){ return; }
    const record = this.history.pop();
    record.effects.forEach((e) => {
      this.current_grid[e.coord.r][e.coord.c] = e.before;
      this.field_view.update(e.coord, e.before);
    });
  }

  export_solution(){
    let result = '';
    this.history.forEach((r) => { result += r.command; });
    return result;
  }

  move_generic(cmd, dy, dx){
    const stringify = (p) => { return p.r + ',' + p.c; }
    const last = this.history[this.history.length - 1];
    const height = this.current_grid.length;
    const width  = this.current_grid[0].length;
    const y = last.position.r + dy, x = last.position.c + dx;
    if(y < 0 || height <= y){ return; }
    if(x < 0 || width  <= x){ return; }
    if(this.current_grid[y][x] == CELL_OBSTACLE){ return; }
    const new_position = new Point(y, x);
    const new_reaches = this.compute_reaches(new_position, last.manipulators);
    let modified = new Set();
    let new_effects = []
    modified.add(stringify(new_position));
    new_effects.push(new Effect(new_position, this.current_grid[y][x], CELL_ROBOT));
    new_reaches.forEach((r) => {
      modified.add(stringify(r));
      new_effects.push(new Effect(r, this.current_grid[r.r][r.c], CELL_REACHING));
    });
    if(!modified.has(stringify(last.position))){
      const r = last.position;
      new_effects.push(new Effect(r, this.current_grid[r.r][r.c], CELL_WRAPPED));
    }
    const old_reaches = this.compute_reaches(last.position, last.manipulators);
    old_reaches.forEach((r) => {
      if(modified.has(stringify(r))){ return; }
      new_effects.push(new Effect(r, this.current_grid[r.r][r.c], CELL_WRAPPED));
    });
    this.push_record(new HistoryRecord(cmd, new_position, new_effects, last.manipulators));
  }

  move_up()   { this.move_generic('W', 1, 0); }
  move_down() { this.move_generic('S', -1, 0); }
  move_left() { this.move_generic('A', 0, -1); }
  move_right(){ this.move_generic('D', 0, 1); }

  rotate_generic(cmd, transform){
    const stringify = (p) => { return p.r + ',' + p.c; }
    const last = this.history[this.history.length - 1];
    const new_manipulators = last.manipulators.map(transform);
    const new_reaches = this.compute_reaches(last.position, new_manipulators);
    let modified = new Set();
    let new_effects = []
    new_reaches.forEach((r) => {
      modified.add(stringify(r));
      new_effects.push(new Effect(r, this.current_grid[r.r][r.c], CELL_REACHING));
    });
    const old_reaches = this.compute_reaches(last.position, last.manipulators);
    old_reaches.forEach((r) => {
      if(modified.has(stringify(r))){ return; }
      new_effects.push(new Effect(r, this.current_grid[r.r][r.c], CELL_WRAPPED));
    });
    this.push_record(new HistoryRecord(cmd, last.position, new_effects, new_manipulators));
  }

  rotate_left() { this.rotate_generic('Q', (v) => { return new Point(v.c, -v.r); }); }
  rotate_right(){ this.rotate_generic('E', (v) => { return new Point(-v.c, v.r); }); }
}
