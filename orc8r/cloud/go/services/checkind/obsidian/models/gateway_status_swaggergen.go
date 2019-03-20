// Code generated by go-swagger; DO NOT EDIT.

package models

// This file was generated by the swagger tool.
// Editing this file might prove futile when you re-run the swagger generate command

import (
	strfmt "github.com/go-openapi/strfmt"

	"github.com/go-openapi/errors"
	"github.com/go-openapi/swag"
)

// GatewayStatus gateway status
// swagger:model gateway_status
type GatewayStatus struct {

	// cert expiration time
	CertExpirationTime int64 `json:"cert_expiration_time,omitempty"`

	// checkin time
	CheckinTime uint64 `json:"checkin_time,omitempty"`

	// hardware id
	HardwareID string `json:"hardware_id,omitempty"`

	// kernel version
	KernelVersion string `json:"kernel_version,omitempty"`

	// kernel versions installed
	KernelVersionsInstalled []string `json:"kernel_versions_installed,omitempty"`

	// meta
	Meta map[string]string `json:"meta,omitempty"`

	// system status
	SystemStatus *SystemStatus `json:"system_status,omitempty"`

	// version
	Version string `json:"version,omitempty"`

	// vpn ip
	VpnIP string `json:"vpn_ip,omitempty"`
}

// Validate validates this gateway status
func (m *GatewayStatus) Validate(formats strfmt.Registry) error {
	var res []error

	if err := m.validateSystemStatus(formats); err != nil {
		res = append(res, err)
	}

	if len(res) > 0 {
		return errors.CompositeValidationError(res...)
	}
	return nil
}

func (m *GatewayStatus) validateSystemStatus(formats strfmt.Registry) error {

	if swag.IsZero(m.SystemStatus) { // not required
		return nil
	}

	if m.SystemStatus != nil {
		if err := m.SystemStatus.Validate(formats); err != nil {
			if ve, ok := err.(*errors.Validation); ok {
				return ve.ValidateName("system_status")
			}
			return err
		}
	}

	return nil
}

// MarshalBinary interface implementation
func (m *GatewayStatus) MarshalBinary() ([]byte, error) {
	if m == nil {
		return nil, nil
	}
	return swag.WriteJSON(m)
}

// UnmarshalBinary interface implementation
func (m *GatewayStatus) UnmarshalBinary(b []byte) error {
	var res GatewayStatus
	if err := swag.ReadJSON(b, &res); err != nil {
		return err
	}
	*m = res
	return nil
}
