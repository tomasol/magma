// @flow
import Highlight from 'react-highlight.js';
import React from 'react';
import {Modal} from 'react-bootstrap';

const WorkflowDefModal = props => {
  return (
    <Modal size="xl" show={props.show} onHide={props.closeModal}>
      <Modal.Header>
        <Modal.Title>Workflow Definition</Modal.Title>
      </Modal.Header>
      <code style={{fontSize: '18px'}}>
        <pre style={{maxHeight: '600px'}}>
          <Highlight language="json">
            {JSON.stringify(props.definition, null, 2)}
          </Highlight>
        </pre>
      </code>
    </Modal>
  );
};

export default WorkflowDefModal;
