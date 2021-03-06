/**
 * Copyright 2004-present Facebook. All Rights Reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

import type {Location} from '../../common/Location';
import type {WithStyles} from '@material-ui/core';

import * as React from 'react';
import Breadcrumbs from '@fbcnms/ui/components/Breadcrumbs';
import {LogEvents, ServerLogger} from '../../common/LoggingUtils';
import {createFragmentContainer, graphql} from 'react-relay';
import {withStyles} from '@material-ui/core/styles';

const styles = _theme => ({
  breadcrumbs: {
    display: 'flex',
    alignItems: 'flex-start',
  },
  locationNameContainer: {
    display: 'flex',
    alignItems: 'center',
    flexWrap: 'wrap',
  },
});

type Props = {
  location: Location,
  onLocationClicked: (locationId: string) => void,
  hideTypes: boolean,
  size?: 'default' | 'small' | 'large',
} & WithStyles<typeof styles>;

const LocationBreadcrumbsTitle = (props: Props) => {
  const {classes, location, onLocationClicked, hideTypes, size} = props;
  return (
    <div className={classes.breadcrumbs}>
      <div className={classes.locationNameContainer}>
        <Breadcrumbs
          breadcrumbs={[...location.locationHierarchy, location].map(l => ({
            id: l.id,
            name: l.name,
            subtext: hideTypes ? null : l.locationType.name,
            onClick: () => onLocationClicked(l.id),
          }))}
          size={size}
          onBreadcrumbClicked={id => {
            ServerLogger.info(LogEvents.LOCATION_CARD_BREADCRUMB_CLICKED, {
              locationId: id,
            });
            onLocationClicked(id);
          }}
        />
      </div>
    </div>
  );
};

LocationBreadcrumbsTitle.defaultProps = {
  size: 'default',
};

export default withStyles(styles)(
  createFragmentContainer(LocationBreadcrumbsTitle, {
    location: graphql`
      fragment LocationBreadcrumbsTitle_location on Location {
        id
        name
        locationType {
          name
        }
        locationHierarchy {
          id
          name
          locationType {
            name
          }
        }
      }
    `,
  }),
);
