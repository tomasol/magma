// Copyright (c) 2004-present Facebook All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// Code generated (@generated) by entc, DO NOT EDIT.

package survey

import (
	"strconv"
	"time"

	"github.com/facebookincubator/ent/dialect/sql"
	"github.com/facebookincubator/symphony/graph/ent/predicate"
)

// ID filters vertices based on their identifier.
func ID(id string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			id, _ := strconv.Atoi(id)
			s.Where(sql.EQ(s.C(FieldID), id))
		},
	)
}

// IDEQ applies the EQ predicate on the ID field.
func IDEQ(id string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			id, _ := strconv.Atoi(id)
			s.Where(sql.EQ(s.C(FieldID), id))
		},
	)
}

// IDNEQ applies the NEQ predicate on the ID field.
func IDNEQ(id string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			id, _ := strconv.Atoi(id)
			s.Where(sql.NEQ(s.C(FieldID), id))
		},
	)
}

// IDIn applies the In predicate on the ID field.
func IDIn(ids ...string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			// if not arguments were provided, append the FALSE constants,
			// since we can't apply "IN ()". This will make this predicate falsy.
			if len(ids) == 0 {
				s.Where(sql.False())
				return
			}
			v := make([]interface{}, len(ids))
			for i := range v {
				v[i], _ = strconv.Atoi(ids[i])
			}
			s.Where(sql.In(s.C(FieldID), v...))
		},
	)
}

// IDNotIn applies the NotIn predicate on the ID field.
func IDNotIn(ids ...string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			// if not arguments were provided, append the FALSE constants,
			// since we can't apply "IN ()". This will make this predicate falsy.
			if len(ids) == 0 {
				s.Where(sql.False())
				return
			}
			v := make([]interface{}, len(ids))
			for i := range v {
				v[i], _ = strconv.Atoi(ids[i])
			}
			s.Where(sql.NotIn(s.C(FieldID), v...))
		},
	)
}

// IDGT applies the GT predicate on the ID field.
func IDGT(id string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			id, _ := strconv.Atoi(id)
			s.Where(sql.GT(s.C(FieldID), id))
		},
	)
}

// IDGTE applies the GTE predicate on the ID field.
func IDGTE(id string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			id, _ := strconv.Atoi(id)
			s.Where(sql.GTE(s.C(FieldID), id))
		},
	)
}

// IDLT applies the LT predicate on the ID field.
func IDLT(id string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			id, _ := strconv.Atoi(id)
			s.Where(sql.LT(s.C(FieldID), id))
		},
	)
}

// IDLTE applies the LTE predicate on the ID field.
func IDLTE(id string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			id, _ := strconv.Atoi(id)
			s.Where(sql.LTE(s.C(FieldID), id))
		},
	)
}

// CreateTime applies equality check predicate on the "create_time" field. It's identical to CreateTimeEQ.
func CreateTime(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.EQ(s.C(FieldCreateTime), v))
		},
	)
}

// UpdateTime applies equality check predicate on the "update_time" field. It's identical to UpdateTimeEQ.
func UpdateTime(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.EQ(s.C(FieldUpdateTime), v))
		},
	)
}

// Name applies equality check predicate on the "name" field. It's identical to NameEQ.
func Name(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.EQ(s.C(FieldName), v))
		},
	)
}

// OwnerName applies equality check predicate on the "owner_name" field. It's identical to OwnerNameEQ.
func OwnerName(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.EQ(s.C(FieldOwnerName), v))
		},
	)
}

// CompletionTimestamp applies equality check predicate on the "completion_timestamp" field. It's identical to CompletionTimestampEQ.
func CompletionTimestamp(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.EQ(s.C(FieldCompletionTimestamp), v))
		},
	)
}

// CreateTimeEQ applies the EQ predicate on the "create_time" field.
func CreateTimeEQ(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.EQ(s.C(FieldCreateTime), v))
		},
	)
}

// CreateTimeNEQ applies the NEQ predicate on the "create_time" field.
func CreateTimeNEQ(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.NEQ(s.C(FieldCreateTime), v))
		},
	)
}

// CreateTimeIn applies the In predicate on the "create_time" field.
func CreateTimeIn(vs ...time.Time) predicate.Survey {
	v := make([]interface{}, len(vs))
	for i := range v {
		v[i] = vs[i]
	}
	return predicate.Survey(
		func(s *sql.Selector) {
			// if not arguments were provided, append the FALSE constants,
			// since we can't apply "IN ()". This will make this predicate falsy.
			if len(vs) == 0 {
				s.Where(sql.False())
				return
			}
			s.Where(sql.In(s.C(FieldCreateTime), v...))
		},
	)
}

// CreateTimeNotIn applies the NotIn predicate on the "create_time" field.
func CreateTimeNotIn(vs ...time.Time) predicate.Survey {
	v := make([]interface{}, len(vs))
	for i := range v {
		v[i] = vs[i]
	}
	return predicate.Survey(
		func(s *sql.Selector) {
			// if not arguments were provided, append the FALSE constants,
			// since we can't apply "IN ()". This will make this predicate falsy.
			if len(vs) == 0 {
				s.Where(sql.False())
				return
			}
			s.Where(sql.NotIn(s.C(FieldCreateTime), v...))
		},
	)
}

// CreateTimeGT applies the GT predicate on the "create_time" field.
func CreateTimeGT(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.GT(s.C(FieldCreateTime), v))
		},
	)
}

// CreateTimeGTE applies the GTE predicate on the "create_time" field.
func CreateTimeGTE(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.GTE(s.C(FieldCreateTime), v))
		},
	)
}

// CreateTimeLT applies the LT predicate on the "create_time" field.
func CreateTimeLT(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.LT(s.C(FieldCreateTime), v))
		},
	)
}

// CreateTimeLTE applies the LTE predicate on the "create_time" field.
func CreateTimeLTE(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.LTE(s.C(FieldCreateTime), v))
		},
	)
}

// UpdateTimeEQ applies the EQ predicate on the "update_time" field.
func UpdateTimeEQ(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.EQ(s.C(FieldUpdateTime), v))
		},
	)
}

// UpdateTimeNEQ applies the NEQ predicate on the "update_time" field.
func UpdateTimeNEQ(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.NEQ(s.C(FieldUpdateTime), v))
		},
	)
}

// UpdateTimeIn applies the In predicate on the "update_time" field.
func UpdateTimeIn(vs ...time.Time) predicate.Survey {
	v := make([]interface{}, len(vs))
	for i := range v {
		v[i] = vs[i]
	}
	return predicate.Survey(
		func(s *sql.Selector) {
			// if not arguments were provided, append the FALSE constants,
			// since we can't apply "IN ()". This will make this predicate falsy.
			if len(vs) == 0 {
				s.Where(sql.False())
				return
			}
			s.Where(sql.In(s.C(FieldUpdateTime), v...))
		},
	)
}

// UpdateTimeNotIn applies the NotIn predicate on the "update_time" field.
func UpdateTimeNotIn(vs ...time.Time) predicate.Survey {
	v := make([]interface{}, len(vs))
	for i := range v {
		v[i] = vs[i]
	}
	return predicate.Survey(
		func(s *sql.Selector) {
			// if not arguments were provided, append the FALSE constants,
			// since we can't apply "IN ()". This will make this predicate falsy.
			if len(vs) == 0 {
				s.Where(sql.False())
				return
			}
			s.Where(sql.NotIn(s.C(FieldUpdateTime), v...))
		},
	)
}

// UpdateTimeGT applies the GT predicate on the "update_time" field.
func UpdateTimeGT(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.GT(s.C(FieldUpdateTime), v))
		},
	)
}

// UpdateTimeGTE applies the GTE predicate on the "update_time" field.
func UpdateTimeGTE(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.GTE(s.C(FieldUpdateTime), v))
		},
	)
}

// UpdateTimeLT applies the LT predicate on the "update_time" field.
func UpdateTimeLT(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.LT(s.C(FieldUpdateTime), v))
		},
	)
}

// UpdateTimeLTE applies the LTE predicate on the "update_time" field.
func UpdateTimeLTE(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.LTE(s.C(FieldUpdateTime), v))
		},
	)
}

// NameEQ applies the EQ predicate on the "name" field.
func NameEQ(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.EQ(s.C(FieldName), v))
		},
	)
}

// NameNEQ applies the NEQ predicate on the "name" field.
func NameNEQ(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.NEQ(s.C(FieldName), v))
		},
	)
}

// NameIn applies the In predicate on the "name" field.
func NameIn(vs ...string) predicate.Survey {
	v := make([]interface{}, len(vs))
	for i := range v {
		v[i] = vs[i]
	}
	return predicate.Survey(
		func(s *sql.Selector) {
			// if not arguments were provided, append the FALSE constants,
			// since we can't apply "IN ()". This will make this predicate falsy.
			if len(vs) == 0 {
				s.Where(sql.False())
				return
			}
			s.Where(sql.In(s.C(FieldName), v...))
		},
	)
}

// NameNotIn applies the NotIn predicate on the "name" field.
func NameNotIn(vs ...string) predicate.Survey {
	v := make([]interface{}, len(vs))
	for i := range v {
		v[i] = vs[i]
	}
	return predicate.Survey(
		func(s *sql.Selector) {
			// if not arguments were provided, append the FALSE constants,
			// since we can't apply "IN ()". This will make this predicate falsy.
			if len(vs) == 0 {
				s.Where(sql.False())
				return
			}
			s.Where(sql.NotIn(s.C(FieldName), v...))
		},
	)
}

// NameGT applies the GT predicate on the "name" field.
func NameGT(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.GT(s.C(FieldName), v))
		},
	)
}

// NameGTE applies the GTE predicate on the "name" field.
func NameGTE(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.GTE(s.C(FieldName), v))
		},
	)
}

// NameLT applies the LT predicate on the "name" field.
func NameLT(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.LT(s.C(FieldName), v))
		},
	)
}

// NameLTE applies the LTE predicate on the "name" field.
func NameLTE(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.LTE(s.C(FieldName), v))
		},
	)
}

// NameContains applies the Contains predicate on the "name" field.
func NameContains(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.Contains(s.C(FieldName), v))
		},
	)
}

// NameHasPrefix applies the HasPrefix predicate on the "name" field.
func NameHasPrefix(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.HasPrefix(s.C(FieldName), v))
		},
	)
}

// NameHasSuffix applies the HasSuffix predicate on the "name" field.
func NameHasSuffix(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.HasSuffix(s.C(FieldName), v))
		},
	)
}

// NameEqualFold applies the EqualFold predicate on the "name" field.
func NameEqualFold(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.EqualFold(s.C(FieldName), v))
		},
	)
}

// NameContainsFold applies the ContainsFold predicate on the "name" field.
func NameContainsFold(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.ContainsFold(s.C(FieldName), v))
		},
	)
}

// OwnerNameEQ applies the EQ predicate on the "owner_name" field.
func OwnerNameEQ(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.EQ(s.C(FieldOwnerName), v))
		},
	)
}

// OwnerNameNEQ applies the NEQ predicate on the "owner_name" field.
func OwnerNameNEQ(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.NEQ(s.C(FieldOwnerName), v))
		},
	)
}

// OwnerNameIn applies the In predicate on the "owner_name" field.
func OwnerNameIn(vs ...string) predicate.Survey {
	v := make([]interface{}, len(vs))
	for i := range v {
		v[i] = vs[i]
	}
	return predicate.Survey(
		func(s *sql.Selector) {
			// if not arguments were provided, append the FALSE constants,
			// since we can't apply "IN ()". This will make this predicate falsy.
			if len(vs) == 0 {
				s.Where(sql.False())
				return
			}
			s.Where(sql.In(s.C(FieldOwnerName), v...))
		},
	)
}

// OwnerNameNotIn applies the NotIn predicate on the "owner_name" field.
func OwnerNameNotIn(vs ...string) predicate.Survey {
	v := make([]interface{}, len(vs))
	for i := range v {
		v[i] = vs[i]
	}
	return predicate.Survey(
		func(s *sql.Selector) {
			// if not arguments were provided, append the FALSE constants,
			// since we can't apply "IN ()". This will make this predicate falsy.
			if len(vs) == 0 {
				s.Where(sql.False())
				return
			}
			s.Where(sql.NotIn(s.C(FieldOwnerName), v...))
		},
	)
}

// OwnerNameGT applies the GT predicate on the "owner_name" field.
func OwnerNameGT(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.GT(s.C(FieldOwnerName), v))
		},
	)
}

// OwnerNameGTE applies the GTE predicate on the "owner_name" field.
func OwnerNameGTE(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.GTE(s.C(FieldOwnerName), v))
		},
	)
}

// OwnerNameLT applies the LT predicate on the "owner_name" field.
func OwnerNameLT(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.LT(s.C(FieldOwnerName), v))
		},
	)
}

// OwnerNameLTE applies the LTE predicate on the "owner_name" field.
func OwnerNameLTE(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.LTE(s.C(FieldOwnerName), v))
		},
	)
}

// OwnerNameContains applies the Contains predicate on the "owner_name" field.
func OwnerNameContains(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.Contains(s.C(FieldOwnerName), v))
		},
	)
}

// OwnerNameHasPrefix applies the HasPrefix predicate on the "owner_name" field.
func OwnerNameHasPrefix(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.HasPrefix(s.C(FieldOwnerName), v))
		},
	)
}

// OwnerNameHasSuffix applies the HasSuffix predicate on the "owner_name" field.
func OwnerNameHasSuffix(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.HasSuffix(s.C(FieldOwnerName), v))
		},
	)
}

// OwnerNameIsNil applies the IsNil predicate on the "owner_name" field.
func OwnerNameIsNil() predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.IsNull(s.C(FieldOwnerName)))
		},
	)
}

// OwnerNameNotNil applies the NotNil predicate on the "owner_name" field.
func OwnerNameNotNil() predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.NotNull(s.C(FieldOwnerName)))
		},
	)
}

// OwnerNameEqualFold applies the EqualFold predicate on the "owner_name" field.
func OwnerNameEqualFold(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.EqualFold(s.C(FieldOwnerName), v))
		},
	)
}

// OwnerNameContainsFold applies the ContainsFold predicate on the "owner_name" field.
func OwnerNameContainsFold(v string) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.ContainsFold(s.C(FieldOwnerName), v))
		},
	)
}

// CompletionTimestampEQ applies the EQ predicate on the "completion_timestamp" field.
func CompletionTimestampEQ(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.EQ(s.C(FieldCompletionTimestamp), v))
		},
	)
}

// CompletionTimestampNEQ applies the NEQ predicate on the "completion_timestamp" field.
func CompletionTimestampNEQ(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.NEQ(s.C(FieldCompletionTimestamp), v))
		},
	)
}

// CompletionTimestampIn applies the In predicate on the "completion_timestamp" field.
func CompletionTimestampIn(vs ...time.Time) predicate.Survey {
	v := make([]interface{}, len(vs))
	for i := range v {
		v[i] = vs[i]
	}
	return predicate.Survey(
		func(s *sql.Selector) {
			// if not arguments were provided, append the FALSE constants,
			// since we can't apply "IN ()". This will make this predicate falsy.
			if len(vs) == 0 {
				s.Where(sql.False())
				return
			}
			s.Where(sql.In(s.C(FieldCompletionTimestamp), v...))
		},
	)
}

// CompletionTimestampNotIn applies the NotIn predicate on the "completion_timestamp" field.
func CompletionTimestampNotIn(vs ...time.Time) predicate.Survey {
	v := make([]interface{}, len(vs))
	for i := range v {
		v[i] = vs[i]
	}
	return predicate.Survey(
		func(s *sql.Selector) {
			// if not arguments were provided, append the FALSE constants,
			// since we can't apply "IN ()". This will make this predicate falsy.
			if len(vs) == 0 {
				s.Where(sql.False())
				return
			}
			s.Where(sql.NotIn(s.C(FieldCompletionTimestamp), v...))
		},
	)
}

// CompletionTimestampGT applies the GT predicate on the "completion_timestamp" field.
func CompletionTimestampGT(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.GT(s.C(FieldCompletionTimestamp), v))
		},
	)
}

// CompletionTimestampGTE applies the GTE predicate on the "completion_timestamp" field.
func CompletionTimestampGTE(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.GTE(s.C(FieldCompletionTimestamp), v))
		},
	)
}

// CompletionTimestampLT applies the LT predicate on the "completion_timestamp" field.
func CompletionTimestampLT(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.LT(s.C(FieldCompletionTimestamp), v))
		},
	)
}

// CompletionTimestampLTE applies the LTE predicate on the "completion_timestamp" field.
func CompletionTimestampLTE(v time.Time) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s.Where(sql.LTE(s.C(FieldCompletionTimestamp), v))
		},
	)
}

// HasLocation applies the HasEdge predicate on the "location" edge.
func HasLocation() predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			step := sql.NewStep(
				sql.From(Table, FieldID),
				sql.To(LocationTable, FieldID),
				sql.Edge(sql.M2O, false, LocationTable, LocationColumn),
			)
			sql.HasNeighbors(s, step)
		},
	)
}

// HasLocationWith applies the HasEdge predicate on the "location" edge with a given conditions (other predicates).
func HasLocationWith(preds ...predicate.Location) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			builder := sql.Dialect(s.Dialect())
			t1 := s.Table()
			t2 := builder.Select(FieldID).From(builder.Table(LocationInverseTable))
			for _, p := range preds {
				p(t2)
			}
			s.Where(sql.In(t1.C(LocationColumn), t2))
		},
	)
}

// HasSourceFile applies the HasEdge predicate on the "source_file" edge.
func HasSourceFile() predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			step := sql.NewStep(
				sql.From(Table, FieldID),
				sql.To(SourceFileTable, FieldID),
				sql.Edge(sql.M2O, false, SourceFileTable, SourceFileColumn),
			)
			sql.HasNeighbors(s, step)
		},
	)
}

// HasSourceFileWith applies the HasEdge predicate on the "source_file" edge with a given conditions (other predicates).
func HasSourceFileWith(preds ...predicate.File) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			builder := sql.Dialect(s.Dialect())
			t1 := s.Table()
			t2 := builder.Select(FieldID).From(builder.Table(SourceFileInverseTable))
			for _, p := range preds {
				p(t2)
			}
			s.Where(sql.In(t1.C(SourceFileColumn), t2))
		},
	)
}

// HasQuestions applies the HasEdge predicate on the "questions" edge.
func HasQuestions() predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			step := sql.NewStep(
				sql.From(Table, FieldID),
				sql.To(QuestionsTable, FieldID),
				sql.Edge(sql.O2M, true, QuestionsTable, QuestionsColumn),
			)
			sql.HasNeighbors(s, step)
		},
	)
}

// HasQuestionsWith applies the HasEdge predicate on the "questions" edge with a given conditions (other predicates).
func HasQuestionsWith(preds ...predicate.SurveyQuestion) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			builder := sql.Dialect(s.Dialect())
			t1 := s.Table()
			t2 := builder.Select(QuestionsColumn).From(builder.Table(QuestionsTable))
			for _, p := range preds {
				p(t2)
			}
			s.Where(sql.In(t1.C(FieldID), t2))
		},
	)
}

// And groups list of predicates with the AND operator between them.
func And(predicates ...predicate.Survey) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s1 := s.Clone().SetP(nil)
			for _, p := range predicates {
				p(s1)
			}
			s.Where(s1.P())
		},
	)
}

// Or groups list of predicates with the OR operator between them.
func Or(predicates ...predicate.Survey) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			s1 := s.Clone().SetP(nil)
			for i, p := range predicates {
				if i > 0 {
					s1.Or()
				}
				p(s1)
			}
			s.Where(s1.P())
		},
	)
}

// Not applies the not operator on the given predicate.
func Not(p predicate.Survey) predicate.Survey {
	return predicate.Survey(
		func(s *sql.Selector) {
			p(s.Not())
		},
	)
}
