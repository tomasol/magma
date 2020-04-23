// @flow
import Pagination from 'react-bootstrap/Pagination';
import React, {Component} from 'react';

class PageCount extends Component {
  constructor(props) {
    super(props);
    this.state = {};
  }

  render() {
    return (
      <Pagination style={{float: 'left'}}>
        <Pagination.Item
          active={this.props.defaultPages === 20}
          onClick={e => {
            const dataSize = this.props.dataSize;
            let size = ~~(dataSize / 20);
            const pagesCount =
              dataSize === 0 ? 0 : dataSize % 20 ? ++size : size;
            this.props.handler(20, pagesCount);
          }}>
          20{' '}
        </Pagination.Item>
        <Pagination.Item
          active={this.props.defaultPages === 50}
          onClick={e => {
            const dataSize = this.props.dataSize;
            let size = ~~(dataSize / 50);
            const pagesCount =
              dataSize === 0 ? 0 : dataSize % 50 ? ++size : size;
            this.props.handler(50, pagesCount);
          }}>
          50{' '}
        </Pagination.Item>
        <Pagination.Item
          active={this.props.defaultPages === 100}
          onClick={e => {
            const dataSize = this.props.dataSize;
            let size = ~~(dataSize / 100);
            const pagesCount =
              dataSize === 0 ? 0 : dataSize % 100 ? ++size : size;
            this.props.handler(100, pagesCount);
          }}>
          100{' '}
        </Pagination.Item>
      </Pagination>
    );
  }
}

export default PageCount;
