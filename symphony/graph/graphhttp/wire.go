// Copyright (c) 2004-present Facebook All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// +build wireinject

package graphhttp

import (
	"net/http"

	"github.com/facebookincubator/symphony/cloud/log"
	"github.com/facebookincubator/symphony/cloud/oc"
	"github.com/facebookincubator/symphony/cloud/server"
	"github.com/facebookincubator/symphony/cloud/server/xserver"
	"github.com/facebookincubator/symphony/graph/viewer"

	"github.com/google/wire"
	"github.com/gorilla/mux"
	"gocloud.dev/server/health"
)

// Config defines the http server config.
type Config struct {
	Tenancy *viewer.MySQLTenancy
	Logger  log.Logger
	Census  oc.Options
}

// NewServer creates a server from config.
func NewServer(cfg Config) (*server.Server, func(), error) {
	wire.Build(
		xserver.ServiceSet,
		xserver.DefaultViews,
		newHealthChecker,
		wire.FieldsOf(new(Config), "Tenancy", "Logger", "Census"),
		newRouter,
		wire.Bind(new(http.Handler), new(*mux.Router)),
		wire.Bind(new(viewer.Tenancy), new(*viewer.MySQLTenancy)),
	)
	return nil, nil, nil
}

func newHealthChecker(tenancy *viewer.MySQLTenancy) []health.Checker {
	return []health.Checker{tenancy}
}
