// @flow
import React, {useEffect, useState} from 'react';
import WorkflowDia from '../../WorkflowExec/DetailsModal/WorkflowDia/WorkflowDia';
import {Button, Modal} from 'react-bootstrap';
import {conductorApiUrlPrefix} from '../../../../constants';
import {HttpClient as http} from '../../../../common/HttpClient';

const DiagramModal = props => {
  const [meta, setMeta] = useState([]);

  useEffect(() => {
    const name = props.wf.split(' / ')[0];
    const version = props.wf.split(' / ')[1];
    http
      .get(conductorApiUrlPrefix + '/metadata/workflow/' + name + '/' + version)
      .then(res => {
        setMeta(res.result);
      });
  }, [props.wf]);

  const handleClose = () => {
    props.modalHandler();
  };

  return (
    <Modal
      size="lg"
      dialogClassName="modal-70w"
      show={props.show}
      onHide={handleClose}>
      <Modal.Header>
        <Modal.Title>Workflow Diagram</Modal.Title>
      </Modal.Header>
      <Modal.Body>
        <WorkflowDia meta={meta} tasks={[]} def={true} />
      </Modal.Body>
      <Modal.Footer>
        <Button variant="secondary" onClick={handleClose}>
          Close
        </Button>
      </Modal.Footer>
    </Modal>
  );
};

export default DiagramModal;
