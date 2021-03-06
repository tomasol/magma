// Copyright (c) 2004-present Facebook All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package resolver

import (
	"context"
	"strconv"
	"testing"
	"time"

	"github.com/facebookincubator/symphony/graph/graphql/models"
	"github.com/facebookincubator/symphony/graph/viewer/viewertest"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

type equipmentSearchDataModels struct {
	locType1 string
	locType2 string
	loc1     string
	loc2     string
	equType  string
}

type woSearchDataModels struct {
	loc1        string
	woType1     string
	assignee1   string
	wo1         string
	owner       string
	installDate time.Time
}

func prepareEquipmentData(ctx context.Context, r *TestResolver, name string, props []*models.PropertyInput) equipmentSearchDataModels {
	mr := r.Mutation()
	locType1, _ := mr.AddLocationType(ctx, models.AddLocationTypeInput{
		Name: name + "loc_type1",
	})
	locType2, _ := mr.AddLocationType(ctx, models.AddLocationTypeInput{
		Name: name + "loc_type2",
	})

	loc1, _ := mr.AddLocation(ctx, models.AddLocationInput{
		Name: name + "loc_inst1",
		Type: locType1.ID,
	})
	loc2, _ := mr.AddLocation(ctx, models.AddLocationInput{
		Name: name + "loc_inst2",
		Type: locType2.ID,
	})
	propType := models.PropertyTypeInput{
		Name: "Owner",
		Type: models.PropertyKindString,
	}
	equType, _ := mr.AddEquipmentType(ctx, models.AddEquipmentTypeInput{
		Name:       name + "eq_type",
		Properties: []*models.PropertyTypeInput{&propType},
	})
	if len(props) != 0 {
		props[0].PropertyTypeID = equType.QueryPropertyTypes().OnlyXID(ctx)
	}
	_, _ = mr.AddEquipment(ctx, models.AddEquipmentInput{
		Name:       name + "eq_inst1",
		Type:       equType.ID,
		Location:   &loc1.ID,
		Properties: props,
	})
	_, _ = mr.AddEquipment(ctx, models.AddEquipmentInput{
		Name:       name + "eq_inst2",
		Type:       equType.ID,
		Location:   &loc2.ID,
		Properties: props,
	})
	return equipmentSearchDataModels{
		locType1.ID,
		locType2.ID,
		loc1.ID,
		loc2.ID,
		equType.ID,
	}
}

func prepareWOData(ctx context.Context, r *TestResolver, name string) woSearchDataModels {
	mr := r.Mutation()
	locType1, _ := mr.AddLocationType(ctx, models.AddLocationTypeInput{
		Name: name + "loc_type1",
	})
	locType2, _ := mr.AddLocationType(ctx, models.AddLocationTypeInput{
		Name: name + "loc_type2",
	})

	loc1, _ := mr.AddLocation(ctx, models.AddLocationInput{
		Name: name + "loc_inst1",
		Type: locType1.ID,
	})
	loc2, _ := mr.AddLocation(ctx, models.AddLocationInput{
		Name: name + "loc_inst2",
		Type: locType2.ID,
	})

	woType1, _ := mr.AddWorkOrderType(ctx, models.AddWorkOrderTypeInput{Name: "wo_type_a"})
	woType2, _ := mr.AddWorkOrderType(ctx, models.AddWorkOrderTypeInput{Name: "wo_type_b"})
	assignee1 := "user1@fb.com"
	assignee2 := "user2@fb.com"
	desc := "random description"

	wo1, _ := mr.AddWorkOrder(ctx, models.AddWorkOrderInput{
		Name:            name + "wo_1",
		Description:     &desc,
		WorkOrderTypeID: woType1.ID,
		LocationID:      &loc1.ID,
		Assignee:        &assignee1,
	})
	_, _ = mr.AddWorkOrder(ctx, models.AddWorkOrderInput{
		Name:            name + "wo_2",
		Description:     &desc,
		WorkOrderTypeID: woType1.ID,
		Assignee:        &assignee1,
	})
	_, _ = mr.AddWorkOrder(ctx, models.AddWorkOrderInput{
		Name:            name + "wo_3",
		Description:     &desc,
		WorkOrderTypeID: woType2.ID,
		LocationID:      &loc1.ID,
		Assignee:        &assignee2,
	})
	_, _ = mr.AddWorkOrder(ctx, models.AddWorkOrderInput{
		Name:            name + "wo_4",
		Description:     &desc,
		WorkOrderTypeID: woType2.ID,
		LocationID:      &loc2.ID,
	})

	installDate := time.Now()
	owner := "owner"
	_, _ = mr.EditWorkOrder(ctx, models.EditWorkOrderInput{
		ID:          wo1.ID,
		Name:        wo1.Name,
		OwnerName:   owner,
		InstallDate: &installDate,
		Status:      models.WorkOrderStatusDone,
		Priority:    models.WorkOrderPriorityHigh,
		LocationID:  &loc1.ID,
		Assignee:    &assignee1,
	})

	return woSearchDataModels{
		loc1.ID,
		woType1.ID,
		assignee1,
		wo1.ID,
		owner,
		installDate,
	}
}

func TestSearchEquipmentByName(t *testing.T) {
	r, err := newTestResolver(t)
	require.NoError(t, err)
	defer r.drv.Close()
	ctx := viewertest.NewContext(r.client)

	mr, qr := r.Mutation(), r.Query()

	locationType, _ := mr.AddLocationType(ctx, models.AddLocationTypeInput{
		Name: "location_type",
	})
	location, _ := mr.AddLocation(ctx, models.AddLocationInput{
		Name: "loCaTIon_name",
		Type: locationType.ID,
	})

	equipmentType, err := mr.AddEquipmentType(ctx, models.AddEquipmentTypeInput{
		Name: "equipment_type",
	})
	require.NoError(t, err)
	eq, _ := mr.AddEquipment(ctx, models.AddEquipmentInput{
		Name:     "EqUiPmEnT",
		Type:     equipmentType.ID,
		Location: &location.ID,
	})

	limit := 10
	equipmentsResult, err := qr.SearchForEntity(ctx, "equip", nil, &limit, nil, nil)
	require.NoError(t, err)

	equipmentResult := equipmentsResult.Edges[0].Node
	assert.Equal(t, equipmentResult.EntityID, eq.ID)
	assert.Equal(t, equipmentResult.EntityType, "equipment")
	assert.Equal(t, equipmentResult.Name, eq.Name)
	assert.Equal(t, equipmentResult.Type, equipmentType.Name)

	require.NoError(t, err)
	_, _ = mr.AddEquipment(ctx, models.AddEquipmentInput{
		Name:     "equipMENT",
		Type:     equipmentType.ID,
		Location: &location.ID,
	})

	equipmentsResult, err = qr.SearchForEntity(ctx, "ment", nil, &limit, nil, nil)
	require.NoError(t, err)

	assert.Len(t, equipmentsResult.Edges, 2)

	locationsResult, err := qr.SearchForEntity(ctx, "cation", nil, &limit, nil, nil)
	require.NoError(t, err)

	locationResult := locationsResult.Edges[0].Node
	assert.Equal(t, locationResult.EntityID, location.ID)
	assert.Equal(t, locationResult.EntityType, "location")
	assert.Equal(t, locationResult.Name, location.Name)
	assert.Equal(t, locationResult.Type, locationType.Name)
}

func TestEquipmentSearch(t *testing.T) {
	r, err := newTestResolver(t)
	require.NoError(t, err)
	defer r.drv.Close()
	ctx := viewertest.NewContext(r.client)

	owner := "Ted"
	prop := models.PropertyInput{
		StringValue: &owner,
	}

	model1 := prepareEquipmentData(ctx, r, "A", []*models.PropertyInput{&prop}) // two locations on same type. each has one equipment.
	model2 := prepareEquipmentData(ctx, r, "B", nil)                            // two locations on same type. each has one equipment.
	/*
		helper: data now is of type:
		loctype1:
			inst1
				eq1 (typeA, name "A_") + prop
			inst2
				eq2 (typeA, name "A_") + prop
		loctype2:
			inst1
				eq1 (typeB, name "B_")
			inst2
				eq2 (typeB, name "B_")
	*/
	qr := r.Query()
	limit := 100
	all, err := qr.EquipmentSearch(ctx, []*models.EquipmentFilterInput{}, &limit)
	require.NoError(t, err)
	require.Len(t, all.Equipment, 4)
	require.Equal(t, all.Count, 4)

	maxDepth := 5
	f1 := models.EquipmentFilterInput{
		FilterType: models.EquipmentFilterTypeLocationInst,
		Operator:   models.FilterOperatorIsOneOf,
		IDSet:      []string{model1.loc1, model2.loc1},
		MaxDepth:   &maxDepth,
	}
	res1, err := qr.EquipmentSearch(ctx, []*models.EquipmentFilterInput{&f1}, &limit)
	require.NoError(t, err)
	require.Len(t, res1.Equipment, 2)
	require.Equal(t, res1.Count, 2)

	f2 := models.EquipmentFilterInput{
		FilterType: models.EquipmentFilterTypeEquipmentType,
		Operator:   models.FilterOperatorIsOneOf,
		IDSet:      []string{model1.equType},
		MaxDepth:   &maxDepth,
	}
	res2, err := qr.EquipmentSearch(ctx, []*models.EquipmentFilterInput{&f1, &f2}, &limit)
	require.NoError(t, err)
	require.Len(t, res2.Equipment, 1)
	require.Equal(t, res2.Count, 1)

	fetchedPropType := res2.Equipment[0].QueryType().QueryPropertyTypes().OnlyX(ctx)
	f3 := models.EquipmentFilterInput{
		FilterType: models.EquipmentFilterTypeProperty,
		Operator:   models.FilterOperatorIs,
		PropertyValue: &models.PropertyTypeInput{
			Name: fetchedPropType.Name,
			Type: models.PropertyKind(fetchedPropType.Type),
		},
		MaxDepth: &maxDepth,
	}
	res3, err := qr.EquipmentSearch(ctx, []*models.EquipmentFilterInput{&f3}, &limit)
	require.NoError(t, err)

	require.Len(t, res3.Equipment, 2)
	require.Equal(t, res3.Count, 2)

	subst := "inst1"
	f4 := models.EquipmentFilterInput{
		FilterType:  models.EquipmentFilterTypeEquipInstName,
		Operator:    models.FilterOperatorContains,
		StringValue: &subst,
		MaxDepth:    &maxDepth,
	}
	res4, err := qr.EquipmentSearch(ctx, []*models.EquipmentFilterInput{&f3, &f4}, &limit)
	require.NoError(t, err)
	require.Len(t, res4.Equipment, 1)
	require.Equal(t, res4.Count, 1)

	f5 := models.EquipmentFilterInput{
		FilterType: models.EquipmentFilterTypeLocationInst,
		Operator:   models.FilterOperatorIsOneOf,
		IDSet:      []string{model2.loc1},
		MaxDepth:   &maxDepth,
	}
	res5, err := qr.EquipmentSearch(ctx, []*models.EquipmentFilterInput{&f3, &f4, &f5}, &limit)
	require.NoError(t, err)
	require.Len(t, res5.Equipment, 0)
	require.Equal(t, res5.Count, 0)
}

func TestUnsupportedEquipmentSearch(t *testing.T) {
	r, err := newTestResolver(t)
	require.NoError(t, err)
	defer r.drv.Close()
	ctx := viewertest.NewContext(r.client)

	qr := r.Query()
	limit := 100

	maxDepth := 5
	f := models.EquipmentFilterInput{
		FilterType: models.EquipmentFilterTypeLocationInst,
		Operator:   models.FilterOperatorContains,
		MaxDepth:   &maxDepth,
	}
	_, err = qr.EquipmentSearch(ctx, []*models.EquipmentFilterInput{&f}, &limit)
	require.Error(t, err)

	f = models.EquipmentFilterInput{
		FilterType: models.EquipmentFilterTypeProperty,
		Operator:   models.FilterOperatorContains,
		MaxDepth:   &maxDepth,
	}
	_, err = qr.EquipmentSearch(ctx, []*models.EquipmentFilterInput{&f}, &limit)
	require.Error(t, err)

	f = models.EquipmentFilterInput{
		FilterType: models.EquipmentFilterTypeEquipmentType,
		Operator:   models.FilterOperatorContains,
		MaxDepth:   &maxDepth,
	}
	_, err = qr.EquipmentSearch(ctx, []*models.EquipmentFilterInput{&f}, &limit)
	require.Error(t, err)
}

func TestQueryEquipmentPossibleProperties(t *testing.T) {
	r, err := newTestResolver(t)
	require.NoError(t, err)
	defer r.drv.Close()
	ctx := viewertest.NewContext(r.client)

	mr, qr := r.Mutation(), r.Query()

	namePropType := models.PropertyTypeInput{
		Name: "Name",
		Type: "string",
	}

	widthPropType := models.PropertyTypeInput{
		Name: "Width",
		Type: "number",
	}

	_, _ = mr.AddEquipmentType(ctx, models.AddEquipmentTypeInput{
		Name:       "example_type_a",
		Properties: []*models.PropertyTypeInput{&namePropType, &widthPropType},
	})

	propDefs, err := qr.PossibleProperties(ctx, models.PropertyEntityEquipment)
	require.NoError(t, err)
	for _, propDef := range propDefs {
		assert.True(t, propDef.Name == "Name" || propDef.Name == "Width")
	}

	assert.Len(t, propDefs, 2)
}

func TestSearchEquipmentByLocation(t *testing.T) {
	r, err := newTestResolver(t)
	require.NoError(t, err)
	defer r.drv.Close()
	ctx := viewertest.NewContext(r.client)

	mr, qr := r.Mutation(), r.Query()
	locType, _ := mr.AddLocationType(ctx, models.AddLocationTypeInput{
		Name: "loc_type1",
	})

	loc1, _ := mr.AddLocation(ctx, models.AddLocationInput{
		Name: "loc_inst1",
		Type: locType.ID,
	})
	loc2, _ := mr.AddLocation(ctx, models.AddLocationInput{
		Name:   "loc_inst2",
		Type:   locType.ID,
		Parent: &loc1.ID,
	})
	eqType, _ := mr.AddEquipmentType(ctx, models.AddEquipmentTypeInput{
		Name: "eq_type",
	})

	_, _ = mr.AddEquipment(ctx, models.AddEquipmentInput{
		Name:     "eq_inst1",
		Type:     eqType.ID,
		Location: &loc1.ID,
	})
	_, _ = mr.AddEquipment(ctx, models.AddEquipmentInput{
		Name:     "eq_inst2",
		Type:     eqType.ID,
		Location: &loc2.ID,
	})

	maxDepth := 2
	limit := 100
	f1 := models.EquipmentFilterInput{
		FilterType: models.EquipmentFilterTypeLocationInst,
		Operator:   models.FilterOperatorIsOneOf,
		IDSet:      []string{loc1.ID},
		MaxDepth:   &maxDepth,
	}
	res1, err := qr.EquipmentSearch(ctx, []*models.EquipmentFilterInput{&f1}, &limit)
	require.NoError(t, err)
	require.Len(t, res1.Equipment, 2)

	f2 := models.EquipmentFilterInput{
		FilterType: models.EquipmentFilterTypeLocationInst,
		Operator:   models.FilterOperatorIsOneOf,
		IDSet:      []string{loc2.ID},
		MaxDepth:   &maxDepth,
	}
	res2, err := qr.EquipmentSearch(ctx, []*models.EquipmentFilterInput{&f2}, &limit)
	require.NoError(t, err)
	require.Len(t, res2.Equipment, 1)
}

func TestSearchEquipmentByDate(t *testing.T) {
	r, err := newTestResolver(t)
	require.NoError(t, err)
	defer r.drv.Close()
	ctx := viewertest.NewContext(r.client)

	mr, qr := r.Mutation(), r.Query()
	locType, _ := mr.AddLocationType(ctx, models.AddLocationTypeInput{
		Name: "loc_type1",
	})

	loc1, _ := mr.AddLocation(ctx, models.AddLocationInput{
		Name: "loc_inst1",
		Type: locType.ID,
	})
	date := "2020-01-01"
	propType := models.PropertyTypeInput{
		Name:        "install_date",
		Type:        models.PropertyKindDate,
		StringValue: &date,
	}
	eqType, _ := mr.AddEquipmentType(ctx, models.AddEquipmentTypeInput{
		Name:       "eq_type",
		Properties: []*models.PropertyTypeInput{&propType},
	})
	date = "2010-01-01"
	ptypeID := eqType.QueryPropertyTypes().OnlyXID(ctx)

	prop1 := models.PropertyInput{
		PropertyTypeID: ptypeID,
		StringValue:    &date,
	}
	e1, _ := mr.AddEquipment(ctx, models.AddEquipmentInput{
		Name:       "eq_inst1",
		Type:       eqType.ID,
		Location:   &loc1.ID,
		Properties: []*models.PropertyInput{&prop1},
	})
	e2, _ := mr.AddEquipment(ctx, models.AddEquipmentInput{
		Name:     "eq_inst2",
		Type:     eqType.ID,
		Location: &loc1.ID,
	})
	_ = e2
	limit := 100
	date = "2015-05-05"
	f1 := models.EquipmentFilterInput{
		FilterType: models.EquipmentFilterTypeProperty,
		Operator:   models.FilterOperatorDateGreaterThan,
		PropertyValue: &models.PropertyTypeInput{
			Name:        "install_date",
			Type:        models.PropertyKindDate,
			StringValue: &date,
		},
	}

	res1, err := qr.EquipmentSearch(ctx, []*models.EquipmentFilterInput{&f1}, &limit)
	require.NoError(t, err)
	require.Len(t, res1.Equipment, 1)
	require.Equal(t, res1.Equipment[0].ID, e2.ID)

	f2 := models.EquipmentFilterInput{
		FilterType: models.EquipmentFilterTypeProperty,
		Operator:   models.FilterOperatorDateLessThan,
		PropertyValue: &models.PropertyTypeInput{
			Name:        "install_date",
			Type:        models.PropertyKindDate,
			StringValue: &date,
		},
	}
	res2, err := qr.EquipmentSearch(ctx, []*models.EquipmentFilterInput{&f2}, &limit)
	require.NoError(t, err)
	require.Len(t, res2.Equipment, 1)
	require.Equal(t, res2.Equipment[0].ID, e1.ID)

	res3, err := qr.EquipmentSearch(ctx, []*models.EquipmentFilterInput{&f1, &f2}, &limit)
	require.NoError(t, err)
	require.Len(t, res3.Equipment, 0)
}

func TestSearchWO(t *testing.T) {
	r, err := newTestResolver(t)
	require.NoError(t, err)
	defer r.drv.Close()
	ctx := viewertest.NewContext(r.client)

	data := prepareWOData(ctx, r, "A")
	/*
		helper: data now is of type:
		wo type a :
			Awo_1: loc1, assignee1. has "owner" and install date
			Awo_2: no loc, assignee1
		wo type b :
			Awo_3: loc1, assignee2
			Awo_4: loc2, no assignee
	*/
	qr := r.Query()
	limit := 100
	all, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{}, &limit)
	require.NoError(t, err)
	require.Len(t, all, 4)

	name := "_1"
	f1 := models.WorkOrderFilterInput{
		FilterType:  models.WorkOrderFilterTypeWorkOrderName,
		Operator:    models.FilterOperatorContains,
		StringValue: &name,
	}
	res1, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{&f1}, &limit)
	require.NoError(t, err)
	require.Len(t, res1, 1)
	require.Equal(t, data.wo1, res1[0].ID)

	status := models.WorkOrderStatusPlanned.String()
	f2 := models.WorkOrderFilterInput{
		FilterType: models.WorkOrderFilterTypeWorkOrderStatus,
		Operator:   models.FilterOperatorIsOneOf,
		IDSet:      []string{status},
	}
	res2, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{&f2}, &limit)
	require.NoError(t, err)
	require.Len(t, res2, 3)

	f3 := models.WorkOrderFilterInput{
		FilterType: models.WorkOrderFilterTypeWorkOrderType,
		Operator:   models.FilterOperatorIsOneOf,
		IDSet:      []string{data.woType1},
	}

	res3, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{&f3}, &limit)
	require.NoError(t, err)
	require.Len(t, res3, 2)

	f4 := models.WorkOrderFilterInput{
		FilterType: models.WorkOrderFilterTypeWorkOrderAssignee,
		Operator:   models.FilterOperatorIsOneOf,
		IDSet:      []string{data.assignee1},
	}
	res4, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{&f4}, &limit)
	require.NoError(t, err)
	require.Len(t, res4, 2)

	f5 := models.WorkOrderFilterInput{
		FilterType: models.WorkOrderFilterTypeWorkOrderLocationInst,
		Operator:   models.FilterOperatorIsOneOf,
		IDSet:      []string{data.loc1},
	}
	res5, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{&f5}, &limit)
	require.NoError(t, err)
	require.Len(t, res5, 2)

	res6, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{&f4, &f5}, &limit)
	require.NoError(t, err)
	require.Len(t, res6, 1)

	f7 := models.WorkOrderFilterInput{
		FilterType: models.WorkOrderFilterTypeWorkOrderOwner,
		Operator:   models.FilterOperatorIsOneOf,
		IDSet:      []string{data.owner},
	}
	res7, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{&f7}, &limit)
	require.NoError(t, err)
	require.Len(t, res7, 1)

	installTimeStr := strconv.FormatInt(data.installDate.Unix(), 10)
	f8 := models.WorkOrderFilterInput{
		FilterType:  models.WorkOrderFilterTypeWorkOrderInstallDate,
		Operator:    models.FilterOperatorIs,
		StringValue: &installTimeStr,
	}
	res8, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{&f8}, &limit)
	require.NoError(t, err)
	require.Len(t, res8, 1)

	now := strconv.FormatInt(time.Now().Unix(), 10)
	f9 := models.WorkOrderFilterInput{
		FilterType:  models.WorkOrderFilterTypeWorkOrderCreationDate,
		Operator:    models.FilterOperatorIs,
		StringValue: &now,
	}
	res9, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{&f9}, &limit)
	require.NoError(t, err)
	require.Len(t, res9, 4)
}

func TestSearchWOByPriority(t *testing.T) {
	r, err := newTestResolver(t)
	require.NoError(t, err)
	defer r.drv.Close()
	ctx := viewertest.NewContext(r.client)
	data := prepareWOData(ctx, r, "B")

	qr := r.Query()
	limit := 100
	all, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{}, &limit)
	require.NoError(t, err)
	require.Len(t, all, 4)

	f1 := models.WorkOrderFilterInput{
		FilterType: models.WorkOrderFilterTypeWorkOrderPriority,
		Operator:   models.FilterOperatorIsOneOf,
		IDSet:      []string{models.WorkOrderPriorityHigh.String()},
	}
	res1, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{&f1}, &limit)
	require.NoError(t, err)
	require.Len(t, res1, 1)
	require.Equal(t, data.wo1, res1[0].ID)

	f2 := models.WorkOrderFilterInput{
		FilterType: models.WorkOrderFilterTypeWorkOrderPriority,
		Operator:   models.FilterOperatorIsOneOf,
		IDSet:      []string{models.WorkOrderPriorityLow.String()},
	}
	res2, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{&f2}, &limit)
	require.NoError(t, err)
	require.Len(t, res2, 0)
}

func TestSearchWOByLocation(t *testing.T) {
	r, err := newTestResolver(t)
	require.NoError(t, err)
	defer r.drv.Close()
	ctx := viewertest.NewContext(r.client)

	data := prepareWOData(ctx, r, "A")
	/*
		helper: data now is of type:
		wo type a :
			Awo_1: loc1, assignee1. has "owner" and install date
			Awo_2: no loc, assignee1
		wo type b :
			Awo_3: loc1, assignee2
			Awo_4: loc2, no assignee
	*/
	qr := r.Query()
	limit := 100
	all, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{}, &limit)
	require.NoError(t, err)
	require.Len(t, all, 4)

	maxDepth := 2
	f1 := models.WorkOrderFilterInput{
		FilterType: models.WorkOrderFilterTypeWorkOrderLocationInst,
		Operator:   models.FilterOperatorIsOneOf,
		IDSet:      []string{data.loc1},
		MaxDepth:   &maxDepth,
	}
	res1, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{&f1}, &limit)
	require.NoError(t, err)
	require.Len(t, res1, 2)

	f2 := models.WorkOrderFilterInput{
		FilterType: models.WorkOrderFilterTypeWorkOrderLocationInst,
		Operator:   models.FilterOperatorIsOneOf,
		IDSet:      []string{"no-id"},
		MaxDepth:   &maxDepth,
	}

	res2, err := qr.WorkOrderSearch(ctx, []*models.WorkOrderFilterInput{&f1, &f2}, &limit)
	require.NoError(t, err)
	require.Empty(t, res2)
}
