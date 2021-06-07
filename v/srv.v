module main

import vweb

struct App {
	vweb.Context
}

fn main() {
	vweb.run(&App{}, 7012)
}

pub fn (mut app App) index() vweb.Result {
	return app.text('Hello')
}
