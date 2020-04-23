// @flow
import * as SRD from '@projectstorm/react-diagrams';
import {CircleEndNodeFactory} from './NodeModels/EndNode/CircleEndNodeFactory';
import {CircleEndPortModel} from './NodeModels/EndNode/CircleEndPortModel';
import {CircleStartNodeFactory} from './NodeModels/StartNode/CircleStartNodeFactory';
import {CircleStartPortModel} from './NodeModels/StartNode/CircleStartPortModel';
import {DecisionNodeFactory} from './NodeModels/DecisionNode/DecisionNodeFactory';
import {DecisionNodePortModel} from './NodeModels/DecisionNode/DecisionNodePortModel';
import {ForkNodeFactory} from './NodeModels/ForkNode/ForkNodeFactory';
import {ForkNodePortModel} from './NodeModels/ForkNode/ForkNodePortModel';
import {JoinNodeFactory} from './NodeModels/JoinNode/JoinNodeFactory';
import {JoinNodePortModel} from './NodeModels/JoinNode/JoinNodePortModel';
import {SimplePortFactory} from './NodeModels/SimplePortFactory';

import {DefaultLabelFactory} from '@projectstorm/react-diagrams';
import {DefaultLinkFactory} from '@projectstorm/react-diagrams';
import {DefaultNodeFactory} from '@projectstorm/react-diagrams';
import {DefaultPortModel} from './NodeModels/DefaultNodeModel/DefaultPortModel';

export class Application {
  activeModel;
  diagramEngine;

  constructor() {
    this.diagramEngine = new SRD.DiagramEngine();

    this.diagramEngine.registerLinkFactory(new DefaultLinkFactory());
    this.diagramEngine.registerLabelFactory(new DefaultLabelFactory());

    this.diagramEngine.registerPortFactory(
      new SimplePortFactory('default', config => new DefaultPortModel()),
    );
    this.diagramEngine.registerPortFactory(
      new SimplePortFactory('start', config => new CircleStartPortModel()),
    );
    this.diagramEngine.registerPortFactory(
      new SimplePortFactory('end', config => new CircleEndPortModel()),
    );
    this.diagramEngine.registerPortFactory(
      new SimplePortFactory('fork', config => new ForkNodePortModel()),
    );
    this.diagramEngine.registerPortFactory(
      new SimplePortFactory('join', config => new JoinNodePortModel()),
    );
    this.diagramEngine.registerPortFactory(
      new SimplePortFactory('decision', config => new DecisionNodePortModel()),
    );

    this.diagramEngine.registerNodeFactory(new DefaultNodeFactory());
    this.diagramEngine.registerNodeFactory(new CircleStartNodeFactory());
    this.diagramEngine.registerNodeFactory(new CircleEndNodeFactory());
    this.diagramEngine.registerNodeFactory(new ForkNodeFactory());
    this.diagramEngine.registerNodeFactory(new JoinNodeFactory());
    this.diagramEngine.registerNodeFactory(new DecisionNodeFactory());
  }

  getActiveDiagram() {
    return this.activeModel;
  }

  getDiagramEngine() {
    return this.diagramEngine;
  }
}
