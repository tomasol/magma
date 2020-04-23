// @flow
import './css/awesomefonts.css';
import './css/bootstrap.min.css';
import './css/mono-blue.min.css';
import './css/neat.css';
import DiagramBuilder from './pages/diagramBuilder/DiagramBuilder';
import React from 'react';
import WorkflowList from './pages/workflowList/WorkflowList';
import buildReducer from './store/reducers/builder';
import bulkReducer from './store/reducers/bulk';
import mountedDeviceReducer from './store/reducers/mountedDevices';
import searchReducer from './store/reducers/searchExecs';
import thunk from 'redux-thunk';
import {BrowserRouter, Route, Switch} from 'react-router-dom';
import {Provider} from 'react-redux';
import {applyMiddleware, combineReducers, compose, createStore} from 'redux';
import {frontendUrlPrefix} from './constants';

const rootReducer = combineReducers({
  bulkReducer,
  searchReducer,
  buildReducer,
  mountedDeviceReducer,
});

const composeEnhancers = window.__REDUX_DEVTOOLS_EXTENSION_COMPOSE__ || compose;

const store = createStore(
  rootReducer,
  composeEnhancers(applyMiddleware(thunk)),
);

function App(props) {
  const hideHeader = () => {
    return props?.setBuilderActive ? props.setBuilderActive(true) : null;
  };

  return (
    <Provider store={store}>
      <BrowserRouter>
        <Switch>
          <Route
            exact
            path={[
              frontendUrlPrefix + '/builder',
              frontendUrlPrefix + '/builder/:name/:version',
            ]}
            render={props => (
              <DiagramBuilder hideHeader={hideHeader} {...props} />
            )}
          />
          <Route
            exact
            path={[
              frontendUrlPrefix + '/:type',
              frontendUrlPrefix + '/:type/:wfid',
              '/',
            ]}
            component={WorkflowList}
          />
        </Switch>
      </BrowserRouter>
    </Provider>
  );
}

export default App;
