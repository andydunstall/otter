package conn

type Conn interface {
	Get(key string) (string, error)
	Put(key string, value string) error
	Delete(key string) error
	Ping() (int64, error)
}
