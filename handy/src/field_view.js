const FIELD_PADDING_SIZE = 4;
const FIELD_CELL_SIZE    = 4;

function rgb_color(r, g, b){ return 'rgb(' + r + ',' + g + ',' + b + ')'; }
const FIELD_COLOR_OBSTACLE  = rgb_color( 64,  64,  64);
const FIELD_COLOR_EMPTY     = rgb_color(255, 255, 255);
const FIELD_COLOR_WRAPPED   = rgb_color(255, 238, 115);
const FIELD_COLOR_REACHING  = rgb_color(237, 179,   0);
const FIELD_COLOR_NEW_MANIP = rgb_color(255, 255,   0);
const FIELD_COLOR_FAST      = rgb_color(153, 102,   0);
const FIELD_COLOR_DRILL     = rgb_color(  0, 204,   0);
const FIELD_COLOR_ROBOT     = rgb_color(197,   0,   0);
const FIELD_COLOR_TABLE = [
  FIELD_COLOR_OBSTACLE,
  FIELD_COLOR_EMPTY,
  FIELD_COLOR_WRAPPED,
  FIELD_COLOR_REACHING,
  FIELD_COLOR_NEW_MANIP,
  FIELD_COLOR_FAST,
  FIELD_COLOR_DRILL,
  FIELD_COLOR_ROBOT
];

class FieldView {
  constructor(canvas){
    this.canvas = canvas;
    this.ctx = canvas.getContext('2d');
    this.height = 0;
    this.width  = 0;
  }
  initialize(descriptor){
    this.ctx.beginPath();
    this.ctx.fillStyle = FIELD_COLOR_OBSTACLE;
    this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
    this.ctx.closePath();
    this.height = descriptor.grid.length;
    this.width  = descriptor.grid[0].length;
    for(let i = 0; i < this.height; ++i){
      for(let j = 0; j < this.width; ++j){
        this.update(new Point(i, j), descriptor.grid[i][j]);
      }
    }
  }
  update(p, c){
    const i = this.height - 1 - p.r;
    const j = p.c;
    const s = FIELD_CELL_SIZE;
    const pad = FIELD_PADDING_SIZE;
    this.ctx.beginPath();
    this.ctx.fillStyle = FIELD_COLOR_TABLE[c];
    this.ctx.fillRect(j * s + pad, i * s + pad, s, s);
    this.ctx.closePath();
  }
}
