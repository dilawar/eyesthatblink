fmt:
	cargo +nightly fmt

fix:
	cargo clippy
	cargo clippy --fix --allow-dirty --allow-staged
