pragma foreign_keys=true;

create table problems (
		id          integer primary key,
		description text    not null,
		content     text    not null
);

create table solutions (
		id         integer   primary key autoincrement,
		problem_id integer   not null,
		score      integer   not null,
		content    text      not null,
		author     text      not null,
		created_at timestamp default (datetime('now', 'localtime')),
		foreign key (problem_id) references problems(id)
);
create index problem_score_index on solutions(problem_id, score);
