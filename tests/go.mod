module github.com/otter-io/otter/tests

go 1.20

require (
	github.com/google/uuid v1.3.1
	github.com/otter-io/otter/client v0.0.0
	github.com/stretchr/testify v1.8.4
)

require (
	github.com/davecgh/go-spew v1.1.1 // indirect
	github.com/pmezard/go-difflib v1.0.0 // indirect
	gopkg.in/yaml.v3 v3.0.1 // indirect
)

replace github.com/otter-io/otter/client v0.0.0 => ../client
