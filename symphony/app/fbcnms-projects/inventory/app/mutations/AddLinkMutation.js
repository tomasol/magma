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
  AddLinkMutationResponse,
  AddLinkMutationVariables,
} from './__generated__/AddLinkMutation.graphql';
import type {MutationCallbacks} from './MutationCallbacks.js';

const mutation = graphql`
  mutation AddLinkMutation($input: AddLinkInput!) {
    addLink(input: $input) {
      ...EquipmentPortsTable_link @relay(mask: false)
    }
  }
`;

export default (
  variables: AddLinkMutationVariables,
  callbacks?: MutationCallbacks<AddLinkMutationResponse>,
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
