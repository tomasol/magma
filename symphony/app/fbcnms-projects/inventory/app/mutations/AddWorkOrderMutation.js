/**
 * Copyright 2004-present Facebook. All Rights Reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

import RelayEnvironment from '../common/RelayEnvironment.js';
import {commitMutation, graphql} from 'react-relay';
import type {
  AddWorkOrderMutationResponse,
  AddWorkOrderMutationVariables,
} from './__generated__/AddWorkOrderMutation.graphql';
import type {MutationCallbacks} from './MutationCallbacks.js';

const mutation = graphql`
  mutation AddWorkOrderMutation($input: AddWorkOrderInput!) {
    addWorkOrder(input: $input) {
      ...WorkOrdersView_workOrder @relay(mask: false)
    }
  }
`;

export default (
  variables: AddWorkOrderMutationVariables,
  callbacks?: MutationCallbacks<AddWorkOrderMutationResponse>,
  updater?: (store: any) => void,
) => {
  const {onCompleted, onError} = callbacks ? callbacks : {};
  commitMutation(RelayEnvironment, {
    mutation,
    variables,
    updater,
    onCompleted,
    onError,
  });
};
