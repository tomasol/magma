// @flow
import {
  CHECKED_WORKFLOWS,
  DATA_SIZE,
  HIERARCHY_NEW_DATA,
  RECEIVE_NEW_DATA,
  UPDATE_LABEL,
  UPDATE_QUERY,
} from '../actions/searchExecs';

const initialState = {
  isFetching: false,
  data: [],
  query: '',
  label: [],
  parents: [],
  children: [],
  size: 0,
  checkedWfs: [0],
};

const reducer = (state = initialState, action) => {
  switch (action.type) {
    case UPDATE_LABEL: {
      let {label} = action;
      label = label ? label : [];
      return {...state, label};
    }
    case UPDATE_QUERY: {
      const {query} = action;
      return {...state, query};
    }
    case RECEIVE_NEW_DATA: {
      const {data} = action;
      return {...state, isFetching: false, data};
    }
    case HIERARCHY_NEW_DATA: {
      const {parents, children} = action;
      return {...state, isFetching: false, parents, children};
    }
    case DATA_SIZE: {
      const {size} = action;
      return {...state, size};
    }
    case CHECKED_WORKFLOWS: {
      const {checkedWfs} = action;
      return {...state, checkedWfs};
    }
    default:
      break;
  }
  return state;
};

export default reducer;
