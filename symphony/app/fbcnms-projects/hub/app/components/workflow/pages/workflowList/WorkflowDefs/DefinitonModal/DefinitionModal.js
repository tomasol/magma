// @flow
import Highlight from 'react-highlight.js';
import React, {useEffect, useState} from 'react';
import {Button, Modal} from 'react-bootstrap';
import {conductorApiUrlPrefix} from '../../../../constants';
import {HttpClient as http} from '../../../../common/HttpClient';

const DefinitionModal = props => {
  const [definition, setDefinition] = useState('');

  useEffect(() => {
    const name = props.wf.split(' / ')[0];
    const version = props.wf.split(' / ')[1];
    http
      .get(conductorApiUrlPrefix + '/metadata/workflow/' + name + '/' + version)
      .then(res => {
        setDefinition(JSON.stringify(res.result, null, 2));
      });
  }, [props.wf]);

  const handleClose = () => {
    props.modalHandler();
  };

  return (
    <Modal size="xl" show={props.show} onHide={handleClose}>
      <Modal.Header>
        <Modal.Title>{props.wf}</Modal.Title>
      </Modal.Header>
      <Modal.Body>
        <code style={{fontSize: '17px'}}>
          <pre style={{maxHeight: '600px'}}>
            <Highlight language="json">{definition}</Highlight>
          </pre>
        </code>
      </Modal.Body>
      <Modal.Footer>
        <Button variant="secondary" onClick={handleClose}>
          Close
        </Button>
      </Modal.Footer>
    </Modal>
  );
};

export default DefinitionModal;
