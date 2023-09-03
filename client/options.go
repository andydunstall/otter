package fuddle

type options struct {
	transport Transport
}

func defaultOptions() *options {
	return &options{
		transport: &TCPTransport{},
	}
}

type Option interface {
	apply(*options)
}

type transportOption struct {
	transport Transport
}

func (o transportOption) apply(opts *options) {
	opts.transport = o.transport
}

func WithTransport(t Transport) Option {
	return transportOption{transport: t}
}
