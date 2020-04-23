// @flow
import Pagination from 'react-bootstrap/Pagination';
import React, {Component} from 'react';

class PageSelect extends Component {
  constructor(props) {
    super(props);
    this.state = {};
  }

  setPages() {
    const output = [];
    const viewedPage = this.props.viewedPage;
    const pagesCount = this.props.count;
    const indent = this.props.indent ? this.props.indent : 2;
    output.push(
      <Pagination.Prev
        key={'prev'}
        disabled={viewedPage === 1 || pagesCount === 0}
        onClick={e => {
          if (viewedPage !== 1 && pagesCount !== 0)
            this.props.handler(viewedPage - 1);
        }}
      />,
    );
    for (let i = 1; i <= pagesCount; i++) {
      if (i >= viewedPage - indent && i <= viewedPage + indent) {
        output.push(
          <Pagination.Item
            key={i}
            active={viewedPage === i}
            onClick={() => this.props.handler(i)}>
            {' '}
            {i}{' '}
          </Pagination.Item>,
        );
      }
    }
    output.push(
      <Pagination.Next
        key={'next'}
        disabled={viewedPage === pagesCount || pagesCount === 0}
        onClick={() => {
          if (viewedPage !== pagesCount && pagesCount !== 0) {
            this.props.handler(viewedPage + 1);
          }
        }}
      />,
    );
    return output;
  }
  render() {
    return <Pagination style={{float: 'right'}}>{this.setPages()}</Pagination>;
  }
}

export default PageSelect;
